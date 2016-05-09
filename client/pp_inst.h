#ifndef PP_CLIENT_INSTANCE_H
#define PP_CLIENT_INSTANCE_H

#include <ppapi/cpp/module.h>
#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/var.h>
#include "src/signal.h"
#include "src/common.h"

class PepperClientInstance : public pp::Instance
{
    public:
        struct Impl;

        PepperClientInstance(PP_Instance instance);

        Signal<operation::Status, std::string> dataReceived;
        void receive();

        Signal<operation::Status, std::string> dataSent;
        void send(const std::string &var);

        Signal<operation::Status> connected;
        void connect();

        Signal<operation::Status> disconnected;
        void disconnect();

        void HandleMessage(const pp::Var &varMessage);

    private:
        ptr<BasePrivImpl> m_impl;
};

struct PepperClientModule : public pp::Module
{
    pp::Instance * CreateInstance(PP_Instance instance);
};

namespace pp
{
    Module * CreateModule();
}
#endif // PP_CLIENT_INSTANCE_H
