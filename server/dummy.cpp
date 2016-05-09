#include "server/dummy.h"
#include <iostream>

using namespace std;

void DummyServer::start()
{
    started(operation::Success);
}

void DummyServer::stop()
{
    stopped(operation::Success);
}

void DummyServer::read(int id)
{
    dataRead[id](operation::Success, "");
}

void DummyServer::write(int           id,
                        const string &str)
{
    dataWritten[id](operation::Success, str);
}
