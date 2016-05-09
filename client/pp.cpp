#include <ppapi/utility/completion_callback_factory.h>
#include <ppapi/cpp/var_dictionary.h>
#include <iostream>
#include "src/common.h"
#include "client/pp.h"
#include "client/pp_inst.h"

using namespace std;
using namespace pp;

struct BaseOperation
{
    BaseOperation(ptr<PepperClient::Impl> impl) : impl(impl) { }
    ptr<PepperClient::Impl> impl;
};

namespace
{
    namespace start
    {
        struct Operation :
            public operation::Deferred<Operation>,
            private BaseOperation
        {
            Operation(ptr<PepperClient::Impl> impl);
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
            Operation(ptr<PepperClient::Impl> impl);
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
            Operation(ptr<PepperClient::Impl> impl);
            void run();
            void step(operation::Status status,
                      const string     &msg);
        };
    }

    namespace write
    {
        struct Operation :
            public operation::Deferred<Operation, string>,
            private BaseOperation
        {
            Operation(ptr<PepperClient::Impl> impl,
                      const string           &message);
            void run();
            void step(operation::Status status,
                      const string     &msg);

            string message;
        };
    }
}

struct PepperClient::Impl : public BasePrivImpl
{
    explicit Impl(ptr<Client>               client,
                  ptr<PepperClientInstance> instance) :
        client(client),
        instance(instance),
        started(false)
    { }

    void start();
    ptr<start::Operation> start_op;
    void stop();
    ptr<stop::Operation> stop_op;
    void read();
    ptr<read::Operation> read_op;
    void write(const string &);
    ptr<write::Operation> write_op;

    ptr<Client>               client;
    ptr<PepperClientInstance> instance;
    bool                      started;
};

static PepperClient::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<PepperClient::Impl *>(p.get());
}

PepperClient::PepperClient(ptr<PepperClientInstance> instance)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<PepperClient>(this), instance);
}

namespace
{
    namespace start
    {
        Operation::Operation(ptr<PepperClient::Impl> impl) :
            operation::Deferred<Operation>(impl->client->started),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->instance->connected.connect(this, &Operation::step);
            impl->instance->connect();
        }

        void Operation::step(operation::Status status)
        {
            impl->instance->connected.disconnect(this, &Operation::step);
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

void PepperClient::Impl::start()
{
    if (!started)
    {
        start_op = make_ptr<start::Operation>(make_non_owning_ptr<Impl>(this));
        start_op->exec();
    }
}

void PepperClient::start()
{
    priv(m_impl)->start();
}

namespace
{
    namespace stop
    {
        Operation::Operation(ptr<PepperClient::Impl> impl) :
            operation::Deferred<Operation>(impl->client->stopped),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->instance->disconnected.connect(this, &Operation::step);
            impl->instance->disconnect();
        }

        void Operation::step(operation::Status status)
        {
            impl->instance->disconnected.disconnect(this, &Operation::step);
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

void PepperClient::Impl::stop()
{
    if (started)
    {
        stop_op = make_ptr<stop::Operation>(make_non_owning_ptr<Impl>(this));
        stop_op->exec();
    }
}

void PepperClient::stop()
{
    priv(m_impl)->stop();
}

namespace
{
    namespace read
    {
        Operation::Operation(ptr<PepperClient::Impl> impl) :
            operation::Deferred<Operation, string>(impl->client->dataRead),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->instance->dataReceived.connect(this, &Operation::step);
            impl->instance->receive();
        }

        void Operation::step(operation::Status status,
                             const string     &msg)
        {
            impl->instance->dataReceived.disconnect(this, &Operation::step);
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
    }
}

void PepperClient::Impl::read()
{
    if (started)
    {
        read_op = make_ptr<read::Operation>(make_non_owning_ptr<Impl>(this));
        read_op->exec();
    }
}

void PepperClient::read()
{
    priv(m_impl)->read();
}

namespace
{
    namespace write
    {
        Operation::Operation(ptr<PepperClient::Impl> impl,
                             const string           &message) :
            operation::Deferred<Operation, string>(impl->client->dataWritten),
            BaseOperation(impl),
            message(message)
        { }

        void Operation::run()
        {
            impl->instance->dataSent.connect(this, &Operation::step);
            impl->instance->send(message);
        }

        void Operation::step(operation::Status status,
                             const string     &msg)
        {
            switch (status)
            {
                case operation::Failure:
                    impl->instance->dataSent.disconnect(this, &Operation::step);
                    fail(msg);
                    break;
                case operation::Timeout:
                    impl->instance->dataSent.disconnect(this, &Operation::step);
                    timeout(msg);
                    break;
                case operation::Success:
                    if (msg == message)
                    {
                        impl->instance->dataSent.disconnect(this, &Operation::step);
                        success(msg);
                    }
                    break;
            }
        }
    }
}

void PepperClient::Impl::write(const string &message)
{
    if (started)
    {
        write_op = make_ptr<write::Operation>(make_non_owning_ptr<Impl>(this), message);
        write_op->exec();
    }
}

void PepperClient::write(const string &message)
{
    priv(m_impl)->write(message);
}
