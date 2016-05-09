#ifndef PP_SERVER_H
#define PP_SERVER_H

#include "server/server.h"

class PepperServerInstance;
class PepperServer : public Server
{
    public:
        struct Impl;

        PepperServer(ptr<PepperServerInstance> instance);

        virtual void start();
        virtual void stop();
        virtual void read(int clientId);
        virtual void write(int                clientId,
                           const std::string &message);

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // PP_SERVER_H
