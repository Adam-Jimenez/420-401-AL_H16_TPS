#include <QtNetwork>
#include <QObject>
#include <iostream>
#include "src/common.h"
#include "server/tcp.h"

using namespace std;

struct BaseOperation
{
    BaseOperation(ptr<TcpServer::Impl> impl) : impl(impl) { }
    ptr<TcpServer::Impl> impl;
};

namespace
{
    namespace start
    {
        struct Operation :
            public operation::Deferred<Operation>,
            private BaseOperation
        {
            Q_OBJECT

            public:
                Operation(ptr<TcpServer::Impl> impl);

            public slots:
                void run();
                void step();
                void error();
        };
    }

    namespace stop
    {
        struct Operation :
            public operation::Deferred<Operation>,
            private BaseOperation
        {
            Q_OBJECT

            public:
                Operation(ptr<TcpServer::Impl> impl);

            public slots:
                void run();
        };
    }

    namespace read
    {
        struct Operation :
            public operation::Deferred<Operation, string>,
            private BaseOperation
        {
            Q_OBJECT

            public:
                enum Step { NOTHING_READ, HEADER_READ, DATA_READ };
                Operation(ptr<TcpServer::Impl> impl,
                          int                  clientId);

            public slots:
                void run();
                void step();
                void error();
                void timedout();

            public:
                int    clientId;
                string data;
                Step   curStep;
                qint32 size;
                QTimer timer;
                int    timeDelay;
        };
    }

    namespace write
    {
        struct Operation :
            public operation::Deferred<Operation, string>,
            private BaseOperation
        {
            Q_OBJECT

            public:
                Operation(ptr<TcpServer::Impl> impl,
                          int                  clientId,
                          const string        &data);

            public slots:
                void run();
                void step(qint64 s);
                void error();

            public:
                int        clientId;
                string     data;
                qint64     size;
                QByteArray array;
        };
    }
}

struct TcpServer::Impl : public BasePrivImpl
{
    explicit Impl(ptr<TcpServer> server,
                  const string  &host,
                  int            port) :
        server(server),
        host(host), port(port),
        started(false)
    {
        net = make_ptr<QTcpServer>();
    }

    void start();
    ptr<start::Operation> start_op;
    void stop();
    ptr<stop::Operation> stop_op;
    void read(int);
    ptr<read::Operation> read_op;
    void write(int,
               const string &);
    ptr<write::Operation> write_op;

    ptr<TcpServer>      server;
    ptr<QTcpServer>     net;
    QList<QTcpSocket *> clients;
    string              host;
    int                 port;
    bool                started;
};

static TcpServer::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<TcpServer::Impl *>(p.get());
}

TcpServer::TcpServer(const string &host,
                     int           port) :
    m_impl(make_ptr<Impl>(this, host, port))
{ }

namespace
{
    namespace start
    {
        Operation::Operation(ptr<TcpServer::Impl> impl) :
            operation::Deferred<Operation>(impl->server->started),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            if (!impl->net->listen(QHostAddress(QString::fromStdString(impl->host)),
                                   static_cast<quint16>(impl->port)))
            {
                fail();
            }
            else
            {
                connect(impl->net.get(), SIGNAL(newConnection()),
                        this, SLOT(step()));
                connect(impl->net.get(), SIGNAL(acceptError(
                                                    QAbstractSocket::SocketError)),
                        this, SLOT(error()));
            }
        }

        void Operation::step()
        {
            QTcpSocket *socket = impl->net->nextPendingConnection();
            socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
            impl->clients << socket;

            // attend que les 2 clients se connectent
            if (impl->clients.size() == 2)
            {
                impl->started = true;
                impl->net->disconnect(this);
                success();
            }
        }

        void Operation::error()
        {
            impl->net->disconnect(this);
            fail();
        }
    }
}

void TcpServer::Impl::start()
{
    if (!started)
    {
        start_op = make_ptr<start::Operation>(make_non_owning_ptr<Impl>(this));
        start_op->exec();
    }
}

void TcpServer::start()
{
    priv(m_impl)->start();
}

namespace
{
    namespace stop
    {
        Operation::Operation(ptr<TcpServer::Impl> impl) :
            operation::Deferred<Operation>(impl->server->stopped),
            BaseOperation(impl) { }

        void Operation::run()
        {
            impl->net->close();
            impl->started = false;
            success();
        }
    }
}

void TcpServer::Impl::stop()
{
    if (started)
    {
        stop_op = make_ptr<stop::Operation>(make_non_owning_ptr<Impl>(this));
        stop_op->exec();
    }
}

void TcpServer::stop()
{
    priv(m_impl)->stop();
}

namespace
{
    namespace read
    {
        Operation::Operation(ptr<TcpServer::Impl> impl,
                             int                  clientId) :
            operation::Deferred<Operation, string>(impl->server->dataRead[clientId]),
            BaseOperation(impl),
            clientId(clientId),
            curStep(NOTHING_READ),
            size(0),
            timeDelay(2000)
        { }

        void Operation::run()
        {
            if ((clientId < 0) || (clientId >= impl->clients.size()))
            {
                fail(data);

                return;
            }

            QTcpSocket *socket = impl->clients[clientId];
            if (socket->state() != QTcpSocket::ConnectedState)
            {
                fail(data);
            }
            else
            {
                connect(socket, SIGNAL(readyRead()),
                        this, SLOT(step()));
                connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                        this, SLOT(error()));
                timer.singleShot(timeDelay, this, SLOT(timedout()));
                // try to read in case we missed a readyRead signal !
                step();
            }
        }

        void Operation::step()
        {
            QTcpSocket *socket = impl->clients[clientId];
            if (socket->state() != QTcpSocket::ConnectedState)
            {
                timer.disconnect();
                socket->disconnect(this);
                fail(data);
            }
            else
            {
                switch (curStep)
                {
                    case NOTHING_READ:
                        if (socket->bytesAvailable() >= 4)
                        {
                            curStep = static_cast<Step>(curStep + 1);
                            QByteArray  tmp = socket->read(4);
                            QDataStream ds(&tmp, QIODevice::ReadWrite);
                            ds >> size;
                        } // else : attend que le signal readyRead soit émis à nouveau
                    // no break .. continue to next label !
                    case HEADER_READ:
                        if ((size != 0) && (socket->bytesAvailable() >= size))
                        {
                            curStep = static_cast<Step>(curStep + 1);
                            QByteArray tmp = socket->read(size);
                            data = string(tmp.data());
                        } // else : attend que le signal readyRead soit émis à nouveau
                    // no break .. continue to next label !
                    case DATA_READ:
                        if ((size != 0) && (static_cast<qint32>(data.size()) == size))
                        {
                            timer.disconnect();
                            socket->disconnect(this);
                            success(data);
                        }
                        break;
                }
            }
        }

        void Operation::error()
        {
            timer.disconnect();
            impl->clients[clientId]->disconnect(this);
            fail(data);
        }

        void Operation::timedout()
        {
            timer.disconnect();
            impl->clients[clientId]->disconnect(this);
            timeout(data);
        }
    }
}

void TcpServer::Impl::read(int clientId)
{
    if (started)
    {
        read_op = make_ptr<read::Operation>(make_non_owning_ptr<Impl>(this), clientId);
        read_op->exec();
    }
}

void TcpServer::read(int clientId)
{
    priv(m_impl)->read(clientId);
}

namespace
{
    namespace write
    {
        Operation::Operation(ptr<TcpServer::Impl> impl,
                             int                  clientId,
                             const string        &data) :
            operation::Deferred<Operation, string>(impl->server->dataWritten[clientId]),
            BaseOperation(impl),
            clientId(clientId),
            data(data)
        { }

        void Operation::run()
        {
            if ((clientId < 0) || (clientId >= impl->clients.size()))
            {
                fail(data);

                return;
            }

            QTcpSocket *socket = impl->clients[clientId];
            if (socket->state() != QTcpSocket::ConnectedState)
            {
                fail(data);
            }
            else
            {
                connect(socket, SIGNAL(bytesWritten(qint64)),
                        this, SLOT(step(qint64)));
                connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                        this, SLOT(error()));

                array.clear();
                array.append(data.c_str());

                QByteArray  temp;
                QDataStream ds(&temp, QIODevice::ReadWrite);
                ds << qint32(array.size());

                temp.append(array);

                size = 0;
                int sent = socket->write(temp);
                if (sent <= 0)
                {
                    socket->disconnect(this);
                    fail(data);
                }
            }
        }

        void Operation::step(qint64 s)
        {
            QTcpSocket *socket = impl->clients[clientId];
            if (s == 4 + array.size())
            {
                socket->disconnect(this);
                success(data);
            }
        }

        void Operation::error()
        {
            impl->clients[clientId]->disconnect(this);
            fail(data);
        }
    }
}

void TcpServer::Impl::write(int           clientId,
                            const string &data)
{
    if (started)
    {
        write_op = make_ptr<write::Operation>(make_non_owning_ptr<Impl>(
                                                  this), clientId, data);
        write_op->exec();
    }
}

void TcpServer::write(int           clientId,
                      const string &data)
{
    priv(m_impl)->write(clientId, data);
}

#include "server_tcp.moc"
