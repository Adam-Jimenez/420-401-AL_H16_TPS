#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "server/server.h"

class TcpServer : public Server
{
    public:
        struct Impl;

        explicit TcpServer(const std::string &host,
                           int                port);

        virtual void start();
        virtual void stop();
        virtual void read(int clientId);
        virtual void write(int                clientId,
                           const std::string &message);

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // TCP_SERVER_H
