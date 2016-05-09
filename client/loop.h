#ifndef CLIENT_LOOP_H
#define CLIENT_LOOP_H

#include "src/common.h"

class Client;
class ClientLoop
{
    public:
        struct Impl;

        ClientLoop(ptr<Client> client);

        void begin();
        bool done() const;
        void end();

        bool cleaned() const;

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // CLIENT_LOOP_H
