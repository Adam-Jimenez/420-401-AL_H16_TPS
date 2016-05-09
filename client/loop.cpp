#include <iostream>
#include <sstream>
#include <map>
#include "client/client.h"
#include "client/loop.h"
#include "src/protocol.h"
#include "src/common.h"
#include "alien/proxy.h"
#include "alien/smart.h"

using namespace std;

namespace
{
    struct ProtocolData
    {
        ptr<Client>                        client;
        int                                playerId;
        ptr<Alien>                         alien;
        int                                turn;
        map< ptr<Alien>, ptr<AlienProxy> > proxys;
        map< int, ptr<Alien> >             aliens;
    };

    namespace start
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<ProtocolData> protocolData) :
                operation::Immediate<Operation>(done),
                protocolData(protocolData)
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
                protocolData->client->started.connect(this, &Operation::readPlayerInfo);
                protocolData->client->start();
            }

            void readPlayerInfo(operation::Status status)
            {
                if (checkStatus(status))
                {
                    protocolData->client->started.disconnect(this,
                                                             &Operation::readPlayerInfo);

                    protocolData->client->dataRead.connect(this,
                                                           &Operation::readPlayerId);
                    protocolData->client->read();
                }
            }

            void readPlayerId(operation::Status status,
                              const string     &data)
            {
                if (checkStatus(status))
                {
                    protocolData->client->dataRead.disconnect(this,
                                                              &Operation::readPlayerId);

                    string       cmd;
                    stringstream ss(data);
                    if (!(ss >> cmd >> protocolData->playerId))
                    {
                        cerr <<
                            "Erreur de communication, pas assez de valeurs reçues ou types invalides."
                             << endl;
                        exit(1);
                    }
                    if (cmd != "playerId")
                    {
                        cerr << "Erreur de communication, attendue playerId." << endl;
                        exit(1);
                    }

                    protocolData->client->dataRead.connect(this,
                                                           &Operation::readNumAliens);
                    protocolData->client->read();
                }
            }

            void readNumAliens(operation::Status status,
                               const string     &data)
            {
                if (checkStatus(status))
                {
                    protocolData->client->dataRead.disconnect(this,
                                                              &Operation::readNumAliens);

                    string       cmd;
                    stringstream ss(data);
                    if (!(ss >> cmd >> numAliens))
                    {
                        cerr <<
                            "Erreur de communication, pas assez de valeurs reçues ou types invalides."
                             << endl;
                        exit(1);
                    }
                    if (cmd != "numAliens")
                    {
                        cerr << "Erreur de communication, attendue numAliens." << endl;
                        exit(1);
                    }

                    readAliensIdLoopPre();
                }
            }

            void readAliensIdLoopPre()
            {
                alienIndex = 0;
                protocolData->client->dataRead.connect(this, &Operation::readAlienId);
                readAliensIdLoop();
            }

            void readAliensIdLoop()
            {
                if (alienIndex != numAliens)
                {
                    protocolData->client->read();
                }
                else
                {
                    readAliensIdLoopPost();
                }
            }

            void readAlienId(operation::Status status,
                             const string     &data)
            {
                if (checkStatus(status))
                {
                    string       cmd;
                    stringstream ss(data);
                    int          alienId;
                    if (!(ss >> cmd >> alienId))
                    {
                        cerr <<
                            "Erreur de communication, pas assez de valeurs reçues ou types invalides."
                             << endl;
                        exit(1);
                    }
                    if (cmd != "alienId")
                    {
                        cerr << "Erreur de communication, attendue alienId." << endl;
                        exit(1);
                    }

                    ptr<Alien> alien =
                        make_ptr<SmartAlien>(static_cast<Alien::Species>(Alien::Grutub +
                                                                         protocolData->
                                                                         playerId),
                                             alienId);
                    ptr<LocalAlienProxy> proxy = make_ptr<LocalAlienProxy>(alien);
                    protocolData->proxys[alien]   = proxy;
                    protocolData->aliens[alienId] = alien;

                    ++alienIndex;
                    readAliensIdLoop();
                }
            }

            void readAliensIdLoopPost()
            {
                protocolData->client->dataRead.disconnect(this, &Operation::readAlienId);
                success();
            }

            int                       alienIndex;
            int                       numAliens;
            ptr<ProtocolData>         protocolData;
            Signal<operation::Status> done;
        };
    }

    namespace cmd
    {
        struct Operation : public operation::Immediate<Operation>
        {
            template <typename Method>
            Operation(ptr<ProtocolData> protocolData,
                      const string     &data,
                      Method            m) :
                operation::Immediate<Operation>(done),
                protocolData(protocolData),
                data(data),
                method(m)
            { }

            void run()
            {
                (this->*method)();
            }

            void checkProtocolData()
            {
                if (protocolData->alien.empty())
                {
                    cerr << "Bug : protocolData->alien est vide!" << endl;
                    exit(1);
                }

                if (protocolData->proxys[protocolData->alien].empty())
                {
                    cerr << "Bug : un proxy sans alien, pas supposé de se produire..." <<
                        endl;
                    exit(1);
                }

            }

            bool checkStatus(operation::Status status,
                             const string     &d=string())
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
                        if (!d.empty() && (d != data))
                        {
                            fail();

                            return false;
                        }
                }

                return true;
            }

            /* move */
            void queryMove()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->moveDecided.connect(this, &cmd::Operation::queryMovePost);
                proxy->queryMove(protocolData->turn);
            }

            void queryMovePost(operation::Status status,
                               Alien::Move       move)
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->moveDecided.disconnect(this, &cmd::Operation::queryMovePost);

                checkStatus(status);

                queryMoveSend(move);
            }

            void queryMoveSend(Alien::Move move)
            {
                stringstream ss;
                ss << Protocol::moveCommand() << " " << protocolData->alien->id() <<
                    " " << move;
                data = ss.str();

                protocolData->client->dataWritten.connect(this,
                                                          &Operation::queryMoveSendPost);
                protocolData->client->write(data);
            }

            void queryMoveSendPost(operation::Status status,
                                   const string     &data)
            {
                protocolData->client->dataWritten.disconnect(this,
                                                             &Operation::queryMoveSendPost);

                if (checkStatus(status, data))
                {
                    success();
                }
            }

            /* eat */
            void queryEat()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->eatDecided.connect(this, &cmd::Operation::queryEatPost);
                proxy->queryEat(protocolData->turn);
            }

            void queryEatPost(operation::Status status,
                              bool              eat)
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->eatDecided.disconnect(this, &cmd::Operation::queryEatPost);

                checkStatus(status);

                queryEatSend(eat);
            }

            void queryEatSend(bool eat)
            {
                stringstream ss;
                ss << Protocol::eatCommand() << " " << protocolData->alien->id() << " " <<
                    eat;
                data = ss.str();

                protocolData->client->dataWritten.connect(this,
                                                          &Operation::queryEatSendPost);
                protocolData->client->write(data);
            }

            void queryEatSendPost(operation::Status status,
                                  const string     &data)
            {
                protocolData->client->dataWritten.disconnect(this,
                                                             &Operation::queryEatSendPost);

                if (checkStatus(status, data))
                {
                    success();
                }
            }

            /* attack */
            void queryAttack()
            {
                checkProtocolData();

                stringstream ss(data);
                int          color;
                int          species;
                if (!(ss >> color >> species))
                {
                    cerr <<
                        "Erreur de communication dans queryAttack, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }

                data = ss.str();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->attackDecided.connect(this, &cmd::Operation::queryAttackPost);
                proxy->queryAttack(protocolData->turn,
                                   static_cast<Alien::Color>(color),
                                   static_cast<Alien::Species>(species));
            }

            void queryAttackPost(operation::Status status,
                                 Alien::Attack     attack)
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->attackDecided.disconnect(this, &cmd::Operation::queryAttackPost);

                checkStatus(status);

                queryAttackSend(attack);
            }

            void queryAttackSend(Alien::Attack attack)
            {
                stringstream ss;
                ss << Protocol::attackCommand() << " " << protocolData->alien->id() <<
                    " " << attack;
                data = ss.str();

                protocolData->client->dataWritten.connect(this,
                                                          &Operation::queryAttackSendPost);
                protocolData->client->write(data);
            }

            void queryAttackSendPost(operation::Status status,
                                     const string     &data)
            {
                protocolData->client->dataWritten.disconnect(this,
                                                             &Operation::queryAttackSendPost);

                if (checkStatus(status, data))
                {
                    success();
                }
            }

            /* color */
            void queryColor()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->colorDecided.connect(this, &cmd::Operation::queryColorPost);
                proxy->queryColor(protocolData->turn);
            }

            void queryColorPost(operation::Status status,
                                Alien::Color      color)
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->colorDecided.disconnect(this, &cmd::Operation::queryColorPost);

                checkStatus(status);

                queryColorSend(color);
            }

            void queryColorSend(Alien::Color color)
            {
                stringstream ss;
                ss << Protocol::colorCommand() << " " << protocolData->alien->id() <<
                    " " << color;
                data = ss.str();

                protocolData->client->dataWritten.connect(this,
                                                          &Operation::queryColorSendPost);
                protocolData->client->write(data);
            }

            void queryColorSendPost(operation::Status status,
                                    const string     &data)
            {
                protocolData->client->dataWritten.disconnect(this,
                                                             &Operation::queryColorSendPost);

                if (checkStatus(status, data))
                {
                    success();
                }
            }

            /* species */
            void querySpecies()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->speciesDecided.connect(this, &cmd::Operation::querySpeciesPost);
                proxy->querySpecies(protocolData->turn);
            }

            void querySpeciesPost(operation::Status status,
                                  Alien::Species    species)
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->speciesDecided.disconnect(this, &cmd::Operation::querySpeciesPost);

                checkStatus(status);

                querySpeciesSend(species);
            }

            void querySpeciesSend(Alien::Species species)
            {
                stringstream ss;
                ss << Protocol::speciesCommand() << " " << protocolData->alien->id() <<
                    " " << species;
                data = ss.str();

                protocolData->client->dataWritten.connect(this,
                                                          &Operation::querySpeciesSendPost);
                protocolData->client->write(data);
            }

            void querySpeciesSendPost(operation::Status status,
                                      const string     &data)
            {
                protocolData->client->dataWritten.disconnect(this,
                                                             &Operation::querySpeciesSendPost);

                if (checkStatus(status, data))
                {
                    success();
                }
            }

            /* turn */
            void infoTurn()
            {
                checkProtocolData();

                stringstream ss(data);
                int          turn;
                if (!(ss >> turn))
                {
                    cerr <<
                        "Erreur de communication dans infoTurn, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }
                protocolData->turn = turn;

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->turnDispatched.connect(this, &cmd::Operation::infoTurnPost);
                proxy->infoTurn(turn);
            }

            void infoTurnPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->turnDispatched.disconnect(this, &cmd::Operation::infoTurnPost);

                success();
            }

            /* spawn */
            void infoSpawn()
            {
                checkProtocolData();

                stringstream ss(data);
                int          babyId;
                if (!(ss >> babyId))
                {
                    cerr <<
                        "Erreur de communication dans infoSpawn, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }
                // cout << "got a baby with id " << babyId << endl;

                ptr<Alien> babyAlien = make_ptr<SmartAlien>(
                    static_cast<Alien::Species>(Alien::Grutub + protocolData->playerId),
                    babyId);
                ptr<LocalAlienProxy> babyProxy = make_ptr<LocalAlienProxy>(babyAlien);
                protocolData->proxys[babyAlien] = babyProxy;
                protocolData->aliens[babyId]    = babyAlien;

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->spawnDispatched.connect(this, &cmd::Operation::infoSpawnPost);
                proxy->infoSpawn(protocolData->turn, babyId);
            }

            void infoSpawnPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->spawnDispatched.disconnect(this, &cmd::Operation::infoSpawnPost);

                success();
            }

            /* status */
            void infoStatus()
            {
                checkProtocolData();

                stringstream ss(data);
                int          x, y, width, height, energy;
                if (!(ss >> x >> y >> width >> height >> energy))
                {
                    cerr <<
                        "Erreur de communication dans infoStatus, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }
                // cout << "got " << x << " " << y << " " << width << " " << height << " " << energy
                // << endl;

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->statusDispatched.connect(this, &cmd::Operation::infoStatusPost);
                proxy->infoStatus(protocolData->turn, x, y, width, height, energy);
            }

            void infoStatusPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->statusDispatched.disconnect(this, &cmd::Operation::infoStatusPost);

                success();
            }

            /* neighboor */
            void infoNeighboor()
            {
                checkProtocolData();

                stringstream ss(data);
                int          x, y, color, species, sleeping, mating, eating;
                if (!(ss >> x >> y >> color >> species >> sleeping >> mating >> eating))
                {
                    cerr <<
                        "Erreur de communication dans infoNeighboor, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->neighboorDispatched.connect(this,
                                                   &cmd::Operation::infoNeighboorPost);
                proxy->infoNeighboor(protocolData->turn,
                                     x, y,
                                     static_cast<Alien::Color>(color),
                                     static_cast<Alien::Species>(species),
                                     static_cast<bool>(sleeping),
                                     static_cast<bool>(mating),
                                     static_cast<bool>(eating));
            }

            void infoNeighboorPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->neighboorDispatched.disconnect(this,
                                                      &cmd::Operation::infoNeighboorPost);

                success();
            }

            /* food */
            void infoFood()
            {
                checkProtocolData();

                stringstream ss(data);
                int          x, y;
                if (!(ss >> x >> y))
                {
                    cerr <<
                        "Erreur de communication dans infoFood, pas assez de valeurs reçues ou types invalides."
                         << endl;
                    exit(1);
                }

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->foodDispatched.connect(this, &cmd::Operation::infoFoodPost);
                proxy->infoFood(protocolData->turn, x, y);
            }

            void infoFoodPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->foodDispatched.disconnect(this, &cmd::Operation::infoFoodPost);

                success();
            }

            /* sleep */
            void infoSleep()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->sleepDispatched.connect(this, &cmd::Operation::infoSleepPost);
                proxy->infoSleep(protocolData->turn);
            }

            void infoSleepPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->sleepDispatched.disconnect(this, &cmd::Operation::infoSleepPost);

                success();
            }

            /* wakeup */
            void infoWakeup()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->wakeupDispatched.connect(this, &cmd::Operation::infoWakeupPost);
                proxy->infoWakeup(protocolData->turn);
            }

            void infoWakeupPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->wakeupDispatched.disconnect(this, &cmd::Operation::infoWakeupPost);

                success();
            }

            /* mate */
            void infoMate()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->mateDispatched.connect(this, &cmd::Operation::infoMatePost);
                proxy->infoMate(protocolData->turn);
            }

            void infoMatePost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->mateDispatched.disconnect(this, &cmd::Operation::infoMatePost);

                success();
            }

            /* win */
            void infoWin()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->winDispatched.connect(this, &cmd::Operation::infoWinPost);
                proxy->infoWin(protocolData->turn);
            }

            void infoWinPost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->winDispatched.disconnect(this, &cmd::Operation::infoWinPost);

                success();
            }

            /* lose */
            void infoLose()
            {
                checkProtocolData();

                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->loseDispatched.connect(this, &cmd::Operation::infoLosePost);
                proxy->infoLose(protocolData->turn);
            }

            void infoLosePost()
            {
                ptr<AlienProxy> proxy = protocolData->proxys[protocolData->alien];
                proxy->loseDispatched.disconnect(this, &cmd::Operation::infoLosePost);

                success();
            }

            ptr<ProtocolData>         protocolData;
            string                    data;
            void                      (cmd::Operation::*method)();
            Signal<operation::Status> done;
        };
    }

    namespace loop
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<ProtocolData> protocolData) :
                operation::Immediate<Operation>(done),
                protocolData(protocolData)
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
                read();
            }

            void read(operation::Status status=operation::Success)
            {
                if (checkStatus(status))
                {
                    protocolData->client->dataRead.connect(this,
                                                           &loop::Operation::readPost);
                    protocolData->client->read();
                }
            }

            void readPost(operation::Status status,
                          const string     &data)
            {
                if (checkStatus(status))
                {
                    protocolData->client->dataRead.disconnect(this,
                                                              &loop::Operation::readPost);

                    stringstream ss(data);
                    string       cmd;
                    int          id;

                    ss >> cmd >> id;
                    if (cmd == Protocol::endCommand())
                    {
                        success();

                        return;
                    }

                    protocolData->alien = protocolData->aliens[id];
                    if (protocolData->alien.empty())
                    {
                        cerr <<
                            "Erreur de communication avec le serveur! L'id est invalide."
                             <<
                            endl;
                        fail();

                        return;
                    }

                    string rem;
                    getline(ss, rem);
                    if (cmd == Protocol::moveCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::queryMove);
                    }
                    else if (cmd == Protocol::eatCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::queryEat);
                    }
                    else if (cmd == Protocol::attackCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::queryAttack);
                    }
                    else if (cmd == Protocol::colorCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::queryColor);
                    }
                    else if (cmd == Protocol::speciesCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::querySpecies);
                    }
                    else if (cmd == Protocol::turnCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoTurn);
                    }
                    else if (cmd == Protocol::spawnCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoSpawn);
                    }
                    else if (cmd == Protocol::statusCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoStatus);
                    }
                    else if (cmd == Protocol::neighboorCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoNeighboor);
                    }
                    else if (cmd == Protocol::foodCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoFood);
                    }
                    else if (cmd == Protocol::sleepCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoSleep);
                    }
                    else if (cmd == Protocol::wakeupCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoWakeup);
                    }
                    else if (cmd == Protocol::mateCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoMate);
                    }
                    else if (cmd == Protocol::winCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoWin);
                    }
                    else if (cmd == Protocol::loseCommand())
                    {
                        cmd_op = make_ptr<cmd::Operation>(protocolData, rem,
                                                          &cmd::Operation::infoLose);
                    }
                    else
                    {
                        cerr << "Got unexpected data : " << data << endl;
                        exit(1);
                    }
                    if (!cmd_op.empty())
                    {
                        cmd_op->done.connect(this, &loop::Operation::read);
                        cmd_op->exec();
                    }
                }
            }

            ptr<cmd::Operation>       cmd_op;
            ptr<ProtocolData>         protocolData;
            Signal<operation::Status> done;
        };
    }
}

struct ClientLoop::Impl : public BasePrivImpl
{
    Impl(ptr<ClientLoop> l,
         ptr<Client>     c) :
        loop(l),
        over(false),
        cleaned(false)
    {
        protocolData         = make_ptr<ProtocolData>();
        protocolData->client = c;
    }

    void begin()
    {
        start_op = make_ptr<start::Operation>(protocolData);
        start_op->done.connect(this, &ClientLoop::Impl::beginPost);
        start_op->exec();
    }

    void beginPost(operation::Status status)
    {
        if (status != operation::Success)
        {
            cerr << "Impossible de se connecter au serveur" << endl;
            exit(1);
        }

        start_op->done.disconnect(this, &ClientLoop::Impl::beginPost);

        run();
    }

    void run()
    {
        loop_op = make_ptr<loop::Operation>(protocolData);
        loop_op->done.connect(this, &ClientLoop::Impl::runPost);
        loop_op->exec();
    }

    void runPost(operation::Status status)
    {
        if (status != operation::Success)
        {
            cerr << "Impossible de se connecter au serveur" << endl;
            exit(1);
        }
        loop_op->done.disconnect(this, &ClientLoop::Impl::runPost);
        over = true;
    }

    bool done() const
    {
        return over;
    }

    void end()
    {
        protocolData->client->stopped.connect(this, &ClientLoop::Impl::endPost);
        protocolData->client->stop();
    }

    void endPost(operation::Status status)
    {
        if (status != operation::Success)
        {
            cerr << "Impossible de terminer la connection " << endl;
            exit(1);
        }
        protocolData->client->stopped.disconnect(this, &ClientLoop::Impl::endPost);
        cleaned = true;
    }

    ptr<ProtocolData>     protocolData;
    ptr<start::Operation> start_op;
    ptr<loop::Operation>  loop_op;
    ptr<ClientLoop>       loop;
    bool                  over;
    bool                  cleaned;
};

static ClientLoop::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<ClientLoop::Impl *>(p.get());
}

ClientLoop::ClientLoop(ptr<Client> client) :
    m_impl(make_ptr<Impl>(make_non_owning_ptr<ClientLoop>(this), client))
{ }

void ClientLoop::begin()
{
    priv(m_impl)->begin();
}

bool ClientLoop::done() const
{
    return priv(m_impl)->done();
}

void ClientLoop::end()
{
    priv(m_impl)->end();
}

bool ClientLoop::cleaned() const
{
    return priv(m_impl)->cleaned;
}
