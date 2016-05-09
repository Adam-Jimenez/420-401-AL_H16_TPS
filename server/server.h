#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <stdexcept>
#include <map>
#include "src/common.h"
#include "src/signal.h"

class Server
{
    public:
        Server();
        virtual ~Server();

        Signal<operation::Status> started;
        virtual void start() = 0;

        Signal<operation::Status> stopped;
        virtual void stop() = 0;

        std::map<int, Signal<operation::Status, std::string> > dataRead;
        virtual void read(int) = 0;

        std::map<int, Signal<operation::Status, std::string> > dataWritten;
        virtual void write(int,
                           const std::string &) = 0;

    private:
        Server(const Server &) { }
        void operator=(const Server &) { }
};
#endif // SERVER_H
