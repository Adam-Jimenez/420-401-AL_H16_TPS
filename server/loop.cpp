#include <iostream>
#include <sstream>
#include <map>
#include "server/server.h"
#include "server/loop.h"
#include "src/common.h"
#include "src/protocol.h"
#include "src/simulation.h"

using namespace std;

namespace
{
    namespace start
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation> sim,
                      ptr<Server>     server) :
                operation::Immediate<Operation>(done),
                sim(sim), server(server)
            { }

            bool checkStatus(operation::Status status)
            {
                switch (status)
                {
                    case operation::Failure:
                        fail();

                        return false;

                    case operation::Timeout:
                        timeout();

                        return false;

                    case operation::Success:
                        break;
                }

                return true;
            }

            void run()
            {
                server->started.connect(this, &Operation::writePlayerInfo);
                server->start();
            }

            void writePlayerInfo(operation::Status status)
            {
                server->started.disconnect(this, &Operation::writePlayerInfo);
                if (checkStatus(status))
                {
                    writePlayerInfoLoopPre();
                }
            }

            void writePlayerInfoLoopPre()
            {
                aliens   = sim->aliens();
                playerId = 0;
                writePlayerInfoLoop();
            }

            void writePlayerInfoLoop()
            {
                if (playerId < sim->numPlayers())
                {
                    writePlayerId();
                }
                else
                {
                    writePlayerInfoLoopPost();
                }
            }

            void writePlayerInfoLoopPost()
            {
                success();
            }

            void writePlayerId()
            {
                stringstream ss;
                ss << "playerId " << playerId;
                server->dataWritten[playerId].connect(this, &Operation::writeNumAliens);
                server->write(playerId, ss.str());
            }

            void writeNumAliens(operation::Status status,
                                const string &)
            {
                server->dataWritten[playerId].disconnect(this,
                                                         &Operation::writeNumAliens);
                if (checkStatus(status))
                {
                    int count = 0;
                    for (vector< ptr<Alien> >::const_iterator it = aliens.begin();
                         it != aliens.end(); ++it)
                    {
                        if ((*it)->realSpecies() ==
                            static_cast<Alien::Species>(Alien::Grutub + playerId))
                        {
                            ++count;
                        }
                    }

                    stringstream ss;
                    ss << "numAliens " << count;
                    server->dataWritten[playerId].connect(this,
                                                          &Operation::writeAliensId);
                    server->write(playerId, ss.str());
                }
            }

            void writeAliensId(operation::Status status,
                               const string &)
            {
                server->dataWritten[playerId].disconnect(this, &Operation::writeAliensId);
                if (checkStatus(status))
                {
                    writeAliensIdLoopPre();
                }
            }

            void writeAliensIdLoopPre()
            {
                alienIt = aliens.begin();
                writeAliensIdLoop();
            }

            void writeAliensIdLoop()
            {
                if (alienIt != aliens.end())
                {
                    if ((*alienIt)->realSpecies() ==
                        static_cast<Alien::Species>(Alien::Grutub + playerId))
                    {
                        writeAlienId();
                    }
                    else
                    {
                        writeAlienIdPost();
                    }
                }
                else
                {
                    writeAliensIdLoopPost();
                }
            }

            void writeAlienId()
            {
                stringstream ss;
                ss << "alienId " << (*alienIt)->id();
                server->dataWritten[playerId].connect(this, &Operation::writeAlienIdPost);
                server->write(playerId, ss.str());
            }

            void writeAlienIdPost(operation::Status status=operation::Success,
                                  const string & =string())
            {

                server->dataWritten[playerId].disconnect(this,
                                                         &Operation::writeAlienIdPost);
                if (checkStatus(status))
                {
                    ++alienIt;
                    writeAliensIdLoop();
                }
            }

            void writeAliensIdLoopPost()
            {
                playerId++;
                writePlayerInfoLoop();
            }

            int                                  playerId;
            vector< ptr<Alien> >                 aliens;
            vector< ptr<Alien> >::const_iterator alienIt;
            ptr<Simulation>                      sim;
            ptr<Server>                          server;
            Signal<operation::Status>            done;
        };
    }

    namespace stop
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation> sim,
                      ptr<Server>     server) :
                operation::Immediate<Operation>(done),
                sim(sim), server(server)
            { }

            bool checkStatus(operation::Status status)
            {
                switch (status)
                {
                    case operation::Failure:
                        fail();

                        return false;

                    case operation::Timeout:
                        timeout();

                        return false;

                    case operation::Success:
                        break;
                }

                return true;
            }

            void run()
            {
                writeEndLoopPre();
            }

            void writeEndLoopPre()
            {
                playerId = 0;
                writeEndLoop();
            }

            void writeEndLoop()
            {
                if (playerId < sim->numPlayers())
                {
                    writeEnd();
                }
                else
                {
                    writeEndLoopPost();
                }
            }

            void writeEndLoopPost()
            {
                server->stopped.connect(this, &Operation::step);
                server->stop();
            }

            void writeEnd()
            {
                stringstream ss;
                ss << Protocol::endCommand() << " " << playerId;
                server->dataWritten[playerId].connect(this, &Operation::writeEndPost);
                server->write(playerId, ss.str());
            }

            void writeEndPost(operation::Status status,
                              const string     &data)
            {
                server->dataWritten[playerId].disconnect(this, &Operation::writeEndPost);
                if (checkStatus(status))
                {
                    playerId++;
                    writeEndLoop();
                }
            }

            void step(operation::Status status)
            {
                if (checkStatus(status))
                {
                    server->stopped.disconnect(this, &Operation::step);

                    sim->stop();
                    success();
                }
            }

            int                       playerId;
            ptr<Simulation>           sim;
            ptr<Server>               server;
            Signal<operation::Status> done;
        };
    }
}

struct ServerLoop::Impl : public BasePrivImpl
{
    Impl(ptr<ServerLoop> prot,
         ptr<Simulation> sim,
         ptr<Server>     server) :
        sim(sim),
        loop(prot),
        server(server),
        cleaned(false)
    {
        count = 0;
        sim->msgPending.connect(this, &ServerLoop::Impl::sendMessage);
    }

    void sendMessage(const std::string &msg)
    {
        // only useful for NACL
        server->write(-1, "qtEval:showMessage(\"" + msg + "\");"); // fails silently with TCP

        server->dataRead[-1].connect(this, &Impl::ackMessage);
        server->read(-1);
    }

    void ackMessage(operation::Status,
                    const std::string &msg)
    {
        server->dataRead[-1].disconnect(this, &Impl::ackMessage);
        sim->msgAcked();
    }

    void begin()
    {
        start_op = make_ptr<start::Operation>(sim, server);
        start_op->done.connect(this, &ServerLoop::Impl::beginPost);
        start_op->exec();
    }

    void beginPost(operation::Status status)
    {
        if (status != operation::Success)
        {
            sim->showMessage("Impossible de lancer le serveur");

            return;
        }
        start_op->done.disconnect(this, &ServerLoop::Impl::beginPost);

        sim->start();
    }

    void end()
    {
        stop_op = make_ptr<stop::Operation>(sim, server);
        stop_op->done.connect(this, &ServerLoop::Impl::endPost);
        stop_op->exec();
    }

    void endPost(operation::Status status)
    {
        cleaned = true;
        if (status != operation::Success)
        {
            sim->showMessage("Impossible de quitter le serveur");
        }

        stop_op->done.disconnect(this, &ServerLoop::Impl::endPost);
    }

    bool done() const
    {
        return sim->started() && sim->over();
    }

    ptr<ServerLoop>       loop;
    ptr<Simulation>       sim;
    ptr<Server>           server;
    ptr<start::Operation> start_op;
    ptr<stop::Operation>  stop_op;
    int                   count;
    bool                  running;
    bool                  cleaned;
};

ServerLoop::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<ServerLoop::Impl *>(p.get());
}

ServerLoop::ServerLoop(ptr<Simulation> sim,
                       ptr<Server>     server)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<ServerLoop>(this), sim, server);
}

void ServerLoop::begin()
{
    priv(m_impl)->begin();
}

bool ServerLoop::done() const
{
    return priv(m_impl)->done();
}

void ServerLoop::end()
{
    priv(m_impl)->end();
}

bool ServerLoop::cleaned() const
{
    return priv(m_impl)->cleaned;
}
