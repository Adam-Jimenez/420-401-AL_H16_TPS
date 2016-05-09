#include "client/pp_inst.h"
#include "client/pp.h"
#include "client/loop.h"
#include "src/protocol.h"
#include "src/common.h"
#include <ppapi/cpp/var_dictionary.h>
#include <queue>

using namespace std;
using namespace pp;

struct BaseOperation
{
    BaseOperation(ptr<PepperClientInstance::Impl> impl) : impl(impl) { }
    ptr<PepperClientInstance::Impl> impl;
};

namespace
{
    namespace receive
    {
        struct Operation :
            public operation::Immediate<Operation, string>,
            private BaseOperation
        {
            Operation(ptr<PepperClientInstance::Impl> impl);
            void run();
            void step();
        };
    }

    namespace connect
    {
        struct Operation :
            public operation::Immediate<Operation>,
            private BaseOperation
        {
            Operation(ptr<PepperClientInstance::Impl> impl);
            void run();
            void step();
        };

    }
}

struct PepperClientInstance::Impl : public BasePrivImpl
{
    Impl(ptr<PepperClientInstance> instance) :
        instance(instance),
        serverReady(false),
        connected(false),
        clientId(-1)
    { }

    void receive();
    ptr<receive::Operation> receive_op;
    void send(const string &data);

    Signal<> dataHandled;
    void handleMessage(const Var &varMessage);

    bool serverReady;
    bool connected;
    void connect();
    ptr<connect::Operation> connect_op;
    void disconnect();

    ptr<PepperClientInstance> instance;
    queue<string>             q;
    int                       clientId;
};

static PepperClientInstance::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<PepperClientInstance::Impl *>(p.get());
}

PepperClientInstance::PepperClientInstance(PP_Instance instance) :
    Instance(instance)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<PepperClientInstance>(this));
}

namespace
{
    namespace receive
    {
        Operation::Operation(ptr<PepperClientInstance::Impl> impl) :
            operation::Immediate<Operation, string>(impl->instance->dataReceived),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            if (!impl->q.empty())
            {
                string msg = impl->q.front();
                impl->q.pop();
                success(msg);
            }
            else
            {
                impl->dataHandled.connect(this, &Operation::step);
            }
        }

        void Operation::step()
        {
            impl->dataHandled.disconnect(this, &Operation::step);
            string msg = impl->q.front();
            impl->q.pop();
            success(msg);
        }
    }
}

void PepperClientInstance::Impl::receive()
{
    receive_op = make_ptr<receive::Operation>(make_non_owning_ptr<Impl>(this));
    receive_op->exec();
}

void PepperClientInstance::receive()
{
    priv(m_impl)->receive();
}

void PepperClientInstance::Impl::send(const string &msg)
{
    VarDictionary dict;
    dict.Set("to", Protocol::labelServer());
    dict.Set("from", Protocol::clientIdToLabel(clientId));
    dict.Set("msg", msg);
    instance->PostMessage(dict);
    instance->dataSent(operation::Success, msg);
}

void PepperClientInstance::send(const string &msg)
{
    priv(m_impl)->send(msg);
}

namespace
{
    namespace connect
    {
        Operation::Operation(ptr<PepperClientInstance::Impl> impl) :
            operation::Immediate<Operation>(impl->instance->connected),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->dataHandled.connect(this, &Operation::step);
            // est-ce que les données sont déjà la ?
            step();
        }

        void Operation::step()
        {
            if ((impl->clientId >= 0) && impl->serverReady)
            {
                impl->dataHandled.disconnect(this, &Operation::step);
                impl->connected = true;
                success();
            }
        }
    }
}

void PepperClientInstance::Impl::connect()
{
    if (!connected)
    {
        connect_op = make_ptr<connect::Operation>(make_non_owning_ptr<Impl>(this));
        connect_op->exec();
    }
}

void PepperClientInstance::connect()
{
    priv(m_impl)->connect();
}

void PepperClientInstance::Impl::disconnect()
{
    if (connected)
    {
        connected = false;
        instance->disconnected(operation::Success);
    }
}

void PepperClientInstance::disconnect()
{
    priv(m_impl)->disconnect();
}

void print(const Var &var)
{
    if (var.is_array()) { cout << "array : "; }
    else if (var.is_bool())
    {
        cout << "bool : " << var.AsBool();
    }
    else if (var.is_dictionary())
    {
        cout << "dict : ";
    }
    else if (var.is_double())
    {
        cout << "double : " << var.AsDouble();
    }
    else if (var.is_int())
    {
        cout << "int : " << var.AsInt();
    }
    else if (var.is_null())
    {
        cout << "null : ";
    }
    else if (var.is_number())
    {
        cout << "number : ";
    }
    else if (var.is_object())
    {
        cout << "obj : ";
    }
    else if (var.is_resource())
    {
        cout << "res : ";
    }
    else if (var.is_string())
    {
        cout << "string : " << var.AsString();
    }
    cout << endl;
}

void PepperClientInstance::Impl::handleMessage(const Var &varMessage)
{
    if (varMessage.is_int())
    {
        // envoyés par JS ou clients
        int32_t cmd = varMessage.AsInt();
        if (static_cast<int32_t>(cmd & 0xffffff00) == Protocol::MsgCommandClientId)
        {
            clientId = static_cast<int>(cmd & 0xff);
            instance->PostMessage(Var(Protocol::MsgCommandClientReady + clientId));
            dataHandled();
        }
        else if (cmd == Protocol::MsgCommandServerReady)
        {
            serverReady = true;
            dataHandled();
        }
    }
    else if (varMessage.is_dictionary())
    {
        VarDictionary dict(varMessage);
        // don't check the to, it's already checked by JS
        // don't check the from, it's always from the server
        Var data = dict.Get("msg");
        if (data.is_string())
        {
            string msg = data.AsString();
            // not sure yet if all data should be notified
            q.push(msg);
            dataHandled();
        }
    }
}

void PepperClientInstance::HandleMessage(const Var &varMessage)
{
    priv(m_impl)->handleMessage(varMessage);
}

ptr<PepperClientModule>   ppmodule;
ptr<PepperClientInstance> ppinstance;
ptr<PepperClient>         client;
ptr<ClientLoop>           loop;

Instance * PepperClientModule::CreateInstance(PP_Instance instance)
{
    ppinstance = make_ptr<PepperClientInstance>(instance);
    client     = make_ptr<PepperClient>(ppinstance);
    loop       = make_ptr<ClientLoop>(client);
    loop->begin();

    return ppinstance.get();
}

namespace pp
{
    Module * CreateModule()
    {
        ppmodule = make_ptr<PepperClientModule>();

        return ppmodule.get();
    }
}
