#ifndef PP_CLIENT_H
#define PP_CLIENT_H

#include "client/client.h"

class PepperClientInstance;
class PepperClient : public Client
{
    public:
        struct Impl;

        PepperClient(ptr<PepperClientInstance> instance);

        virtual void start();
        virtual void stop();
        virtual void read();
        virtual void write(const std::string &);

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // PP_CLIENT_H
