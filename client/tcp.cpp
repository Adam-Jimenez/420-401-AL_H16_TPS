#include <QtNetwork>
#include <QObject>
#include <iostream>
#include "src/common.h"
#include "client/tcp.h"

using namespace std;

struct BaseOperation
{
    BaseOperation(ptr<TcpClient::Impl> impl) : impl(impl) { }
    ptr<TcpClient::Impl> impl;
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
                Operation(ptr<TcpClient::Impl> impl);

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
                Operation(ptr<TcpClient::Impl> impl);

            public slots:
                void run();
                void step();
                void error();
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
                Operation(ptr<TcpClient::Impl> impl);

            public slots:
                void run();
                void step();
                void error();

            public:
                string data;
                Step   curStep;
                qint32 size;
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
                Operation(ptr<TcpClient::Impl> impl,
                          const string        &data);

            public slots:
                void run();
                void step(qint64 s);
                void error();

            public:
                string     data;
                qint64     size;
                QByteArray array;
        };
    }
}

struct TcpClient::Impl : public BasePrivImpl
{
    explicit Impl(ptr<TcpClient> client,
                  const string  &host,
                  int            port) :
        client(client),
        host(host), port(port),
        started(false)
    {
        net = make_ptr<QTcpSocket>();
    }

    void start();
    ptr<start::Operation> start_op;
    void stop();
    ptr<stop::Operation> stop_op;
    void read();
    ptr<read::Operation> read_op;
    void write(const string &);
    ptr<write::Operation> write_op;

    ptr<TcpClient>  client;
    ptr<QTcpSocket> net;
    string          host;
    int             port;
    bool            started;
};

static TcpClient::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<TcpClient::Impl *>(p.get());
}

TcpClient::TcpClient(const string &host,
                     int           port)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<TcpClient>(this), host, port);
}

namespace
{
    namespace start
    {
        Operation::Operation(ptr<TcpClient::Impl> impl) :
            operation::Deferred<Operation>(impl->client->started),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            QTcpSocket *socket = impl->net.get();
            connect(socket, SIGNAL(connected()),
                    this, SLOT(step()));
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(error()));
            socket->connectToHost(QHostAddress(QString::fromStdString(impl->host)),
                                  static_cast<quint16>(impl->port));
        }

        void Operation::step()
        {
            impl->net->setSocketOption(QAbstractSocket::LowDelayOption, 1);
            impl->started = true;
            impl->net->disconnect(this);
            success();
        }

        void Operation::error()
        {
            impl->net->disconnect(this);
            fail();
        }
    }
}

void TcpClient::Impl::start()
{
    if (!started)
    {
        start_op = make_ptr<start::Operation>(make_non_owning_ptr<Impl>(this));
        start_op->exec();
    }
}

void TcpClient::start()
{
    priv(m_impl)->start();
}

namespace
{
    namespace stop
    {
        Operation::Operation(ptr<TcpClient::Impl> impl) :
            operation::Deferred<Operation>(impl->client->stopped),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            QTcpSocket *socket = impl->net.get();
            connect(socket, SIGNAL(disconnected()),
                    this, SLOT(step()));
            connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                    this, SLOT(error()));
            socket->disconnectFromHost();
        }

        void Operation::step()
        {
            impl->started = false;
            impl->net->disconnect(this);
            success();
        }

        void Operation::error()
        {
            impl->net->disconnect(this);
            fail();
        }
    }
}

void TcpClient::Impl::stop()
{
    if (started)
    {
        stop_op = make_ptr<stop::Operation>(make_non_owning_ptr<Impl>(this));
        stop_op->exec();
    }
}

void TcpClient::stop()
{
    priv(m_impl)->stop();
}

namespace
{
    namespace read
    {
        Operation::Operation(ptr<TcpClient::Impl> impl) :
            operation::Deferred<Operation, string>(impl->client->dataRead),
            BaseOperation(impl),
            curStep(NOTHING_READ),
            size(0)
        { }

        void Operation::run()
        {
            QTcpSocket *socket = impl->net.get();
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
                // try to read in case we missed a readyRead signal !
                step();
            }
        }

        void Operation::step()
        {
            QTcpSocket *socket = impl->net.get();
            if (socket->state() != QTcpSocket::ConnectedState)
            {
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
                            socket->disconnect(this);
                            success(data);
                        }
                        break;
                }
            }
        }

        void Operation::error()
        {
            impl->net->disconnect(this);
            fail(data);
        }
    }
}

void TcpClient::Impl::read()
{
    if (started)
    {
        read_op = make_ptr<read::Operation>(make_non_owning_ptr<Impl>(this));
        read_op->exec();
    }
}

void TcpClient::read()
{
    priv(m_impl)->read();
}

namespace
{
    namespace write
    {
        Operation::Operation(ptr<TcpClient::Impl> impl,
                             const string        &data) :
            operation::Deferred<Operation, string>(impl->client->dataWritten),
            BaseOperation(impl),
            data(data)
        { }

        void Operation::run()
        {
            QTcpSocket *socket = impl->net.get();
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
            QTcpSocket *socket = impl->net.get();
            if (s == 4 + array.size())
            {
                socket->disconnect(this);
                success(data);
            }
        }

        void Operation::error()
        {
            impl->net->disconnect(this);
            fail(data);
        }
    }
}

void TcpClient::Impl::write(const string &data)
{
    if (started)
    {
        write_op = make_ptr<write::Operation>(make_non_owning_ptr<Impl>(this), data);
        write_op->exec();
    }
}

void TcpClient::write(const string &data)
{
    priv(m_impl)->write(data);
}

#include "client_tcp.moc"
