#include "server/pp_inst.h"
#include "server/pp.h"
#include "server/loop.h"
#include "src/simulation.h"
#include "src/protocol.h"
#include "src/common.h"
#include "src/protocol.h"
#include "alien/aliens.h"

#include <QtWidgets>
#include <ppapi/cpp/var_dictionary.h>
#include <ppapi/utility/completion_callback_factory.h>
#include <queue>
#include <map>

using namespace std;
using namespace pp;

struct BaseOperation
{
    BaseOperation(ptr<PepperServerInstance::Impl> impl) : impl(impl) { }
    ptr<PepperServerInstance::Impl> impl;
};

namespace
{
    namespace listen
    {
        struct Operation :
            public operation::Immediate<Operation>,
            private BaseOperation
        {
            Operation(ptr<PepperServerInstance::Impl> impl);
            void run();
            void step();
        };
    }

    namespace receive
    {
        struct Operation :
            public operation::Immediate<Operation, string>,
            private BaseOperation
        {
            Operation(ptr<PepperServerInstance::Impl> impl,
                      int                             clientId);
            void run();
            void step();
            int clientId;
        };
    }
}

struct PepperServerInstance::Impl : public BasePrivImpl
{
    Impl(ptr<PepperServerInstance> instance) :
        instance(instance),
        listening(false),
        numExpectedClients(-1)
    {
        qs[-1] = queue<string>(); // special
    }

    void receive(int clientId);
    ptr<receive::Operation> receive_op;
    void send(int           clientId,
              const string &data);

    bool listening;
    void listen();
    ptr<listen::Operation> listen_op;
    void close();

    void handleMessage(const Var &varMessage);

    ptr<PepperServerInstance> instance;
    map<int, queue<string> >  qs;
    map<int, Signal<> >       dataHandled;
    Signal<>                  listenHandled;
    int                       numExpectedClients;
};

static PepperServerInstance::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<PepperServerInstance::Impl *>(p.get());
}

PepperServerInstance::PepperServerInstance(PP_Instance instance) :
    QPepperInstance(instance)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<PepperServerInstance>(this));
}

namespace
{
    namespace receive
    {
        Operation::Operation(ptr<PepperServerInstance::Impl> impl,
                             int                             clientId) :
            operation::Immediate<Operation,
                                 string>(impl->instance->dataReceived[clientId]),
            BaseOperation(impl),
            clientId(clientId)
        { }

        void Operation::run()
        {
            if (impl->qs.find(clientId) == impl->qs.end())
            {
                fail(string());

                return;
            }

            if (!impl->qs[clientId].empty())
            {
                string s = impl->qs[clientId].front();
                impl->qs[clientId].pop();
                success(s);
            }
            else
            {
                impl->dataHandled[clientId].connect(this, &Operation::step);
            }
        }

        void Operation::step()
        {
            impl->dataHandled[clientId].disconnect(this, &Operation::step);
            string s = impl->qs[clientId].front();
            impl->qs[clientId].pop();
            success(s);
        }
    }
}

void PepperServerInstance::Impl::receive(int clientId)
{
    if (listening)
    {
        receive_op = make_ptr<receive::Operation>(make_non_owning_ptr<Impl>(
                                                      this), clientId);
        receive_op->exec();
    }
}

void PepperServerInstance::receive(int clientId)
{
    priv(m_impl)->receive(clientId);
}

void PepperServerInstance::Impl::send(int           clientId,
                                      const string &msg)
{
    if (listening)
    {
        if (clientId == -1)
        {
            // special case ! msg to JS
            instance->PostMessage(Var(msg));
        }
        else
        {
            VarDictionary dict;
            dict.Set("to", Protocol::clientIdToLabel(clientId));
            dict.Set("from", Protocol::labelServer());
            dict.Set("msg", msg);
            instance->PostMessage(dict);
            instance->dataSent[clientId](operation::Success, msg);
        }
    }
}

void PepperServerInstance::send(int           clientId,
                                const string &msg)
{
    priv(m_impl)->send(clientId, msg);
}

namespace
{
    namespace listen
    {
        Operation::Operation(ptr<PepperServerInstance::Impl> impl) :
            operation::Immediate<Operation>(impl->instance->listening),
            BaseOperation(impl)
        { }

        void Operation::run()
        {
            impl->listenHandled.connect(this, &Operation::step);
            // est-ce que les données sont déjà la ?
            step();
        }

        void Operation::step()
        {
            if ((impl->numExpectedClients >= 0) &&
                (impl->qs.size() == impl->numExpectedClients + 1))
            {
                impl->listenHandled.disconnect(this, &Operation::step);
                impl->instance->PostMessage(Var(Protocol::MsgCommandServerReady));
                impl->listening = true;
                success();
            }
        }
    }
}

void PepperServerInstance::Impl::listen()
{
    if (!listening)
    {
        listen_op = make_ptr<listen::Operation>(make_non_owning_ptr<Impl>(this));
        listen_op->exec();
    }
}

void PepperServerInstance::listen()
{
    priv(m_impl)->listen();
}

void PepperServerInstance::Impl::close()
{
    if (listening)
    {
        listening = false;
        instance->closed(operation::Success);
    }
}

void PepperServerInstance::close()
{
    priv(m_impl)->close();
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

void PepperServerInstance::Impl::handleMessage(const Var &varMessage)
{
    if (varMessage.is_int())
    {
        // envoyés par JS ou clients
        int32_t cmd = varMessage.AsInt();
        if (static_cast<int32_t>(cmd & 0xffffff00) == Protocol::MsgCommandNumClients)
        {
            numExpectedClients = static_cast<int>(cmd & 0xff);
            listenHandled();
        }
        else if (static_cast<int32_t>(cmd & 0xffffff00) ==
                 Protocol::MsgCommandClientReady)
        {
            int clientId = static_cast<int>(cmd & 0xff);
            if (qs.find(clientId) == qs.end())
            {
                qs[clientId]          = queue<string>();
                dataHandled[clientId] = Signal<>();
            }
        }
        else if (static_cast<int32_t>(cmd) == Protocol::MsgMessageAck)
        {
            qs[-1].push("");
            dataHandled[-1]();
        }
    }
    else if (varMessage.is_dictionary())
    {
        VarDictionary dict(varMessage);
        // don't check the to, it's already checked by JS
        string from     = dict.Get("from").AsString();
        int    clientId = Protocol::clientLabelToId(from);
        if (qs.find(clientId) != qs.end())
        {
            Var data = dict.Get("msg");
            if (data.is_string())
            {
                string msg = data.AsString();
                // not sure yet if all data should be notified
                qs[clientId].push(msg);
                dataHandled[clientId]();
            }
        }
        else
        {
            cerr << "received a message from an unknown client : " << clientId <<
                ", ignoring." << endl;
        }
    }
}

void PepperServerInstance::HandleMessage(const Var &varMessage)
{
    QPepperInstance::HandleMessage(varMessage);
    priv(m_impl)->handleMessage(varMessage);
}

ptr<PepperServerModule>   ppmodule;
ptr<PepperServerInstance> ppinstance;
ptr<PepperServer>         server;
ptr<ServerLoop>           loop;
ptr<Simulation>           sim;

Instance * PepperServerModule::CreateInstance(PP_Instance instance)
{
    ppinstance = make_ptr<PepperServerInstance>(instance);
    server     = make_ptr<PepperServer>(ppinstance);

    return ppinstance.get();
}

void app_init(int,
              char **)
{

    bool local            = false;
    int  numPlayers       = 2;
    int  numOfEachSpecies = 25;
    int  width            = 40;
    int  height           = 30;
    sim = make_ptr<Simulation>(width, height);

    for (int j = 0; j < numOfEachSpecies; ++j)
    {
        ptr<Alien> alien = make_ptr<AlienUqomua>();
        sim->addLocalAlien(alien);
    }

    for (int j = 0; j < numOfEachSpecies; ++j)
    {
        ptr<Alien> alien = make_ptr<AlienOg>();
        sim->addLocalAlien(alien);
    }

    for (int j = 0; j < numOfEachSpecies; ++j)
    {
        ptr<Alien> alien = make_ptr<AlienYuhq>();
        sim->addLocalAlien(alien);
    }

    for (int j = 0; j < numOfEachSpecies; ++j)
    {
        ptr<Alien> alien = make_ptr<AlienEpoe>();
        sim->addLocalAlien(alien);
    }

    for (int i = 0; i < numPlayers; ++i)
    {
        if (!local)
        {
            for (int j = 0; j < numOfEachSpecies; ++j)
            {
                ptr<Alien> alien =
                    make_ptr<SmartAlien>(static_cast<Alien::Species>(Alien::Grutub + i));
                sim->addRemoteAlien(server, i, alien);
            }
        }
        else
        {
            for (int j = 0; j < numOfEachSpecies; ++j)
            {
                ptr<Alien> alien = make_ptr<SmartAlien>(
                    i == 0 ? Alien::Grutub : Alien::Owa);
                sim->addLocalAlien(alien);
            }
        }
    }

    loop = make_ptr<ServerLoop>(sim, server);
    loop->begin();
}

void app_exit()
{
    loop->end();

    // processe les derniers événements qui pourraient rester dans la queue !
    qApp->processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);
}

namespace pp
{
    Module * CreateModule()
    {
        qWidgetsRegisterAppFunctions(app_init, app_exit);
        ppmodule = make_ptr<PepperServerModule>();

        return ppmodule.get();
    }
}