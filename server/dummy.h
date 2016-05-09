#ifndef DUMMY_SERVER_H
#define DUMMY_SERVER_H

#include "server/server.h"

class DummyServer : public Server
{
    public:
        virtual void start();
        virtual void stop();
        virtual void read(int clientId);
        virtual void write(int                clientId,
                           const std::string &message);
};
#endif // DUMMY_SERVER_H
