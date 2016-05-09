#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include "client/client.h"

class TcpClient : public Client
{
    public:
        struct Impl;

        explicit TcpClient(const std::string &host,
                           int                port);

        virtual void start();
        virtual void stop();
        virtual void read();
        virtual void write(const std::string &);

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // TCP_CLIENT_H
