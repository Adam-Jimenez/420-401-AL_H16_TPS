#ifndef PP_SERVER_INSTANCE_H
#define PP_SERVER_INSTANCE_H

#include <QtGui/QPepperInstance>
#include <QtGui/QPepperModule>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var.h>
#include <map>
#include "src/signal.h"
#include "src/common.h"

class PepperServerInstance : public QPepperInstance
{
    public:
        struct Impl;

        PepperServerInstance(PP_Instance instance);

        std::map< int, Signal<operation::Status, std::string> > dataReceived;
        void receive(int clientId);

        std::map<int, Signal<operation::Status, std::string> > dataSent;
        void send(int                clientId,
                  const std::string &var);

        void listen();
        Signal<operation::Status> listening;

        void close();
        Signal<operation::Status> closed;

        void HandleMessage(const pp::Var &varMessage);

    private:
        ptr<BasePrivImpl> m_impl;
};

struct PepperServerModule : public QPepperModule
{
    pp::Instance * CreateInstance(PP_Instance instance);
};

namespace pp
{
    Module * CreateModule();
}
#endif // PP_SERVER_INSTANCE_H
