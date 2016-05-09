#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include "src/common.h"
#include "src/signal.h"

class Client
{
    public:
        Client();
        virtual ~Client();

        Signal<operation::Status> started;
        virtual void start() = 0;

        Signal<operation::Status> stopped;
        virtual void stop() = 0;

        Signal<operation::Status, std::string> dataRead;
        virtual void read() = 0;

        Signal<operation::Status, std::string> dataWritten;
        virtual void write(const std::string &) = 0;

    private:
        Client(const Client &) { }
        void operator=(const Client &) { }
};
#endif // CLIENT_H
