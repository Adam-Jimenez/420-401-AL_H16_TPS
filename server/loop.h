#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include "src/common.h"

class Simulation;
class Server;
class ServerLoop
{
    public:
        struct Impl;

        ServerLoop(ptr<Simulation> sim,
                   ptr<Server>     client);

        void begin();
        bool done() const;
        void end();

        bool cleaned() const;

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // SERVER_PROTOCOL_H
