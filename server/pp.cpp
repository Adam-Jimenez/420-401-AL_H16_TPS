#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/cpp/var_dictionary.h>
#include <iostream>
#include "src/common.h"
#include "server/pp.h"
#include "server/pp_inst.h"

using namespace std;
using namespace pp;

struct BaseOperation
{
    BaseOperation(ptr<PepperServer::Impl> impl) : impl(impl) { }
    ptr<PepperServer::Impl> impl;
};

namespace
{
    namespace start
    {
        struct Operation :
            public operation::Deferred<Operation>,
            private BaseOperation
        {
            Operation(ptr<PepperServer::Impl> impl);
            void run();
            void step(operation::Status status);
        };
    }

    namespace stop
    {
        struct Operation :
            public operation::Deferred<Operation>,
            private BaseOperation
        {
            Operation(ptr<PepperServer::Impl> impl);
            void run();
            void step(operation::Status status);
        };
    }

    namespace read
    {
        struct Operation :
            public operation::Deferred<Operation, string>,
            private BaseOperation
        {
            Operation(ptr<PepperServer::Impl> impl,
                      int                     clientId);
            void run();
            void step(operation::Status status,
                      const string     &msg);
            void _timeout(int32_t);

            int                                      clientId;
            int                                      delayTimeout;
            pp::CompletionCallbackFactory<Operation> cb_factory;
        };
    }

    namespace write
    {
        struct Operation :
            public operation::Deferred<Operation, string>,
            private BaseOperation
        {
            Operation(ptr<PepperServer::Impl> impl,
                      int                     clientId,
                      const string           &message);
            void run();
            void step(operation::Status status,
                      const string     &msg);

            int    clientId;
            string message;
        };
    }

}

struct PepperServer::Impl : public BasePrivImpl
{
    explicit Impl(ptr<Server>               server,
                  ptr<PepperServerInstance> instance) :
        server(server),
        instance(instance),
        started(false)
    { }

    void start();
    ptr<start::Operation> start_op;
    void stop();
    ptr<stop::Operation> stop_op;
    void read(int);
    ptr<read::Operation> read_op;
    void write(int,
               const string &);
    ptr<write::Operation> write_op;

    ptr<Server>               server;
    ptr<PepperServerInstance> instance;
    bool                      started;
};

static PepperServer::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<PepperServer::Impl *>(p.get());
}

PepperServer::PepperServer(ptr<PepperServerInstance> instance)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<PepperServer>(this), instance);
}

namespace
{
    namespace start
    {
        Operation::Operation(ptr<PepperServer::Impl> impl) :
            operation::Deferred<Operation>(impl->server->started),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->instance->listening.connect(this, &Operation::step);
            impl->instance->listen();
        }

        void Operation::step(operation::Status status)
        {
            impl->instance->listening.disconnect(this, &Operation::step);
            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    timeout();
                    break;
                case operation::Success:
                    impl->started = true;
                    success();
                    break;
            }
        }
    }
}

void PepperServer::Impl::start()
{
    if (!started)
    {
        start_op = make_ptr<start::Operation>(make_non_owning_ptr<Impl>(this));
        start_op->exec();
    }
}

void PepperServer::start()
{
    priv(m_impl)->start();
}

namespace
{
    namespace stop
    {
        Operation::Operation(ptr<PepperServer::Impl> impl) :
            operation::Deferred<Operation>(impl->server->stopped),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->instance->closed.connect(this, &Operation::step);
            impl->instance->close();
        }

        void Operation::step(operation::Status status)
        {
            impl->instance->closed.disconnect(this, &Operation::step);
            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    timeout();
                    break;
                case operation::Success:
                    impl->started = false;
                    success();
                    break;
            }
        }
    }
}

void PepperServer::Impl::stop()
{
    if (started)
    {
        stop_op = make_ptr<stop::Operation>(make_non_owning_ptr<Impl>(this));
        stop_op->exec();
    }
}

void PepperServer::stop()
{
    priv(m_impl)->stop();
}

namespace
{
    namespace read
    {
        Operation::Operation(ptr<PepperServer::Impl> impl,
                             int                     clientId) :
            operation::Deferred<Operation, string>(impl->server->dataRead[clientId]),
            BaseOperation(impl),
            clientId(clientId),
            delayTimeout(2000),
            cb_factory(this)
        { }

        void Operation::run()
        {
            // ce callback pourrait laisser des leaks, mais pas de méthodes CancelAll dans la
            // version stable :(
            if (clientId >= 0)
            {
                pp::Module::Get()->core()->CallOnMainThread(delayTimeout,
                                                            cb_factory.NewCallback(&
                                                                                   Operation
                                                                                   ::
                                                                                   _timeout));
            }
            impl->instance->dataReceived[clientId].connect(this, &Operation::step);
            impl->instance->receive(clientId);
        }

        void Operation::step(operation::Status status,
                             const string     &msg)
        {
            impl->instance->dataReceived[clientId].disconnect(this, &Operation::step);
            switch (status)
            {
                case operation::Failure:
                    fail(msg);
                    break;
                case operation::Timeout:
                    timeout(msg);
                    break;
                case operation::Success:
                    success(msg);
                    break;
            }
        }

        void Operation::_timeout(int32_t)
        {
            impl->instance->dataReceived[clientId].disconnect(this, &Operation::step);
            timeout("");
        }
    }
}

void PepperServer::Impl::read(int clientId)
{
    if (started)
    {
        read_op = make_ptr<read::Operation>(make_non_owning_ptr<Impl>(this), clientId);
        read_op->exec();
    }
}

void PepperServer::read(int clientId)
{
    priv(m_impl)->read(clientId);
}

namespace
{
    namespace write
    {
        Operation::Operation(ptr<PepperServer::Impl> impl,
                             int                     clientId,
                             const string           &message) :
            operation::Deferred<Operation, string>(impl->server->dataWritten[clientId]),
            BaseOperation(impl),
            clientId(clientId),
            message(message)
        { }

        void Operation::run()
        {
            impl->instance->dataSent[clientId].connect(this, &Operation::step);
            impl->instance->send(clientId, message);
        }

        void Operation::step(operation::Status status,
                             const string     &msg)
        {
            switch (status)
            {
                case operation::Failure:
                    impl->instance->dataSent[clientId].disconnect(this, &Operation::step);
                    fail(msg);
                    break;
                case operation::Timeout:
                    impl->instance->dataSent[clientId].disconnect(this, &Operation::step);
                    timeout(msg);
                    break;
                case operation::Success:
                    if (msg == message)
                    {
                        impl->instance->dataSent[clientId].disconnect(this,
                                                                      &Operation::step);
                        success(msg);
                    }
                    break;
            }
        }
    }
}

void PepperServer::Impl::write(int           clientId,
                               const string &message)
{
    if (started)
    {
        write_op = make_ptr<write::Operation>(make_non_owning_ptr<Impl>(
                                                  this), clientId, message);
        write_op->exec();
    }
}

void PepperServer::write(int           clientId,
                         const string &message)
{
    priv(m_impl)->write(clientId, message);
}
