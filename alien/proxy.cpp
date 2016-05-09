#include <cstdlib>
#include <sstream>
#include "src/simulation.h"
#include "src/protocol.h"
#include "server/server.h"
#include "alien/proxy.h"

using namespace std;

struct AlienProxy::Impl : public BasePrivImpl
{
    Impl(ptr<AlienProxy> proxy,
         ptr<Alien>      alien) :
        proxy(proxy),
        alien(alien) { }

    ptr<Alien> alienClone() const
    {
        return alien->clone();
    }

    /* move */
    void preQueryMove(int)
    {
        queryMove();
    }

    virtual void queryMove()
    {
        Alien::Move move = alien->queryMove();
        proxy->moveDecided(operation::Success, move);
    }

    void postQueryMove(operation::Status,
                       Alien::Move) { }

    /* eat */
    void preQueryEat(int eatingTurn)
    {
        turn = eatingTurn;
        queryEat();
    }

    virtual void queryEat()
    {
        bool eat = alien->queryEat();
        proxy->eatDecided(operation::Success, eat);
    }

    void postQueryEat(operation::Status,
                      bool eat)
    {
        if (eat)
        {
            addEnergy(turn, Simulation::EatingBonus);
            alien->setEatingTurn(turn);
        }
    }

    /* attack */
    void preQueryAttack(int,
                        Alien::Color   color,
                        Alien::Species species)
    {
        queryAttack(color, species);
    }

    virtual void queryAttack(Alien::Color   color,
                             Alien::Species species)
    {
        Alien::Attack attack = alien->queryAttack(color, species);
        proxy->attackDecided(operation::Success, attack);
    }

    void postQueryAttack(operation::Status,
                         Alien::Attack) { }

    /* color */
    void preQueryColor(int)
    {
        queryColor();
    }

    virtual void queryColor()
    {
        Alien::Color color = alien->queryColor();
        proxy->colorDecided(operation::Success, color);
    }

    void postQueryColor(operation::Status status,
                        Alien::Color      color)
    {
        alien->setColor(color);
    }

    /* species */
    void preQuerySpecies(int)
    {
        querySpecies();
    }

    virtual void querySpecies()
    {
        Alien::Species species = alien->querySpecies();
        proxy->speciesDecided(operation::Success, species);
    }

    void postQuerySpecies(operation::Status,
                          Alien::Species species)
    {
        alien->setSpecies(species);
    }

    /* turn */
    void preInfoTurn(int turn)
    {
        infoTurn(turn);
    }

    virtual void infoTurn(int /*turn*/)
    {
        proxy->turnDispatched();
    }

    void postInfoTurn() { }

    /* spawn */
    void preInfoSpawn(int,
                      int babyId)
    {
        infoSpawn(babyId);
    }

    virtual void infoSpawn(int babyId)
    {
        alien->infoSpawn(babyId);
        proxy->spawnDispatched();
    }

    void postInfoSpawn() { }

    /* status */
    void preInfoStatus(int /*turn*/,
                       int x,
                       int y,
                       int width,
                       int height,
                       int energy)
    {
        infoStatus(x, y, width, height, energy);
    }

    virtual void infoStatus(int x,
                            int y,
                            int width,
                            int height,
                            int energy)
    {
        alien->infoStatus(x, y, width, height, energy);
        proxy->statusDispatched();
    }

    void postInfoStatus() { }

    /* neighboor */
    void preInfoNeighboor(int,
                          int            x,
                          int            y,
                          Alien::Color   color,
                          Alien::Species species,
                          bool           sleeping,
                          bool           mating,
                          bool           eating)
    {
        infoNeighboor(x, y, color, species, sleeping, mating, eating);
    }

    virtual void infoNeighboor(int            x,
                               int            y,
                               Alien::Color   color,
                               Alien::Species species,
                               bool           sleeping,
                               bool           mating,
                               bool           eating)
    {
        alien->infoNeighboor(x, y, color, species, sleeping, mating, eating);
        proxy->neighboorDispatched();
    }

    void postInfoNeighboor() { }

    /* food */
    void preInfoFood(int,
                     int x,
                     int y)
    {
        infoFood(x, y);
    }

    virtual void infoFood(int x,
                          int y)
    {
        alien->infoFood(x, y);
        proxy->foodDispatched();
    }

    void postInfoFood() { }

    /* sleep */
    void preInfoSleep(int sleepingTurn)
    {
        alien->setSleepingTurn(sleepingTurn);
        infoSleep();
    }

    virtual void infoSleep()
    {
        alien->infoSleep();
        proxy->sleepDispatched();
    }

    void postInfoSleep() { }

    /* wakeup */
    void preInfoWakeup(int)
    {
        alien->setSleepingTurn(-1);
        alien->setEatingTurn(-1);
        alien->setMatingTurn(-1);
        infoWakeup();
    }

    virtual void infoWakeup()
    {
        alien->infoWakeup();
        proxy->wakeupDispatched();
    }

    void postInfoWakeup() { }

    /* mate */
    void preInfoMate(int turn)
    {
        alien->setMatingTurn(turn);
        infoMate();
    }

    virtual void infoMate()
    {
        alien->infoMate();
        proxy->mateDispatched();
    }

    void postInfoMate() { }

    /* win */
    void preInfoWin(int /*turn*/)
    {
        infoWin();
    }

    virtual void infoWin()
    {
        alien->infoWin();
        proxy->winDispatched();
    }

    void postInfoWin() { }

    /* lose */
    void preInfoLose(int /*turn*/)
    {
        infoLose();
    }

    virtual void infoLose()
    {
        alien->infoLose();
        proxy->loseDispatched();
    }

    void postInfoLose() { }

    /* addEnergy */
    void addEnergy(int,
                   int delta)
    {
        int energy = alien->energy();
        energy = min<int>(energy + delta, Simulation::MaxEnergy);
        alien->setEnergy(energy);

        postAddEnergy();
    }

    void postAddEnergy()
    {
        proxy->energyAdded();
    }

    /* removeEnergy */
    void removeEnergy(int turn,
                      int delta)
    {
        int energy = alien->energy();
        energy = max(energy - delta, 0);
        alien->setEnergy(energy);

        if (energy == 0)
        {
            proxy->sleepDispatched.connect(this, &AlienProxy::Impl::postRemoveEnergy);
            proxy->infoSleep(turn);
        }
        else
        {
            postRemoveEnergy();
        }
    }

    void postRemoveEnergy()
    {
        proxy->sleepDispatched.disconnect(this, &AlienProxy::Impl::postRemoveEnergy);
        proxy->energyRemoved();
    }

    ptr<AlienProxy> proxy;
    ptr<Alien>      alien;
    int             turn;
};

static AlienProxy::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<AlienProxy::Impl *>(p.get());
}

AlienProxy::AlienProxy(ptr<BasePrivImpl> impl) :
    m_impl(impl)
{
    moveDecided.connect(priv(m_impl), &AlienProxy::Impl::postQueryMove);
    eatDecided.connect(priv(m_impl), &AlienProxy::Impl::postQueryEat);
    attackDecided.connect(priv(m_impl), &AlienProxy::Impl::postQueryAttack);
    colorDecided.connect(priv(m_impl), &AlienProxy::Impl::postQueryColor);
    speciesDecided.connect(priv(m_impl), &AlienProxy::Impl::postQuerySpecies);

    spawnDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoSpawn);
    statusDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoStatus);
    neighboorDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoNeighboor);
    foodDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoFood);
    sleepDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoSleep);
    wakeupDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoWakeup);
    mateDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoMate);
    winDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoWin);
    loseDispatched.connect(priv(m_impl), &AlienProxy::Impl::postInfoLose);
}

AlienProxy::~AlienProxy()
{ }

ptr<Alien> AlienProxy::alien() const
{
    return priv(m_impl)->alien;
}

void AlienProxy::queryMove(int turn)
{
    priv(m_impl)->preQueryMove(turn);
}

void AlienProxy::queryEat(int turn)
{
    priv(m_impl)->preQueryEat(turn);
}

void AlienProxy::queryAttack(int            turn,
                             Alien::Color   color,
                             Alien::Species species)
{
    priv(m_impl)->preQueryAttack(turn, color, species);
}

void AlienProxy::queryColor(int turn)
{
    priv(m_impl)->preQueryColor(turn);
}

void AlienProxy::querySpecies(int turn)
{
    priv(m_impl)->preQuerySpecies(turn);
}

void AlienProxy::infoTurn(int turn) const
{
    priv(m_impl)->preInfoTurn(turn);
}

void AlienProxy::infoSpawn(int turn,
                           int babyId) const
{
    priv(m_impl)->preInfoSpawn(turn, babyId);
}

void AlienProxy::infoStatus(int turn,
                            int x,
                            int y,
                            int width,
                            int height,
                            int energy)
{
    priv(m_impl)->preInfoStatus(turn, x, y, width, height, energy);
}

void AlienProxy::infoNeighboor(int            turn,
                               int            x,
                               int            y,
                               Alien::Color   color,
                               Alien::Species species,
                               bool           sleeping,
                               bool           mating,
                               bool           eating)
{
    priv(m_impl)->preInfoNeighboor(turn, x, y, color, species, sleeping, mating, eating);
}

void AlienProxy::infoFood(int turn,
                          int x,
                          int y)
{
    priv(m_impl)->preInfoFood(turn, x, y);
}

void AlienProxy::infoSleep(int turn)
{
    priv(m_impl)->preInfoSleep(turn);
}

void AlienProxy::infoWakeup(int turn)
{
    priv(m_impl)->preInfoWakeup(turn);
}

void AlienProxy::infoMate(int turn)
{
    priv(m_impl)->preInfoMate(turn);
}

void AlienProxy::infoWin(int turn)
{
    priv(m_impl)->preInfoWin(turn);
}

void AlienProxy::infoLose(int turn)
{
    priv(m_impl)->preInfoLose(turn);
}

void AlienProxy::addEnergy(int turn,
                           int delta)
{
    priv(m_impl)->addEnergy(turn, delta);
}

void AlienProxy::removeEnergy(int turn,
                              int delta)
{
    priv(m_impl)->removeEnergy(turn, delta);
}

struct LocalAlienProxy::Impl : public AlienProxy::Impl
{
    Impl(ptr<LocalAlienProxy> proxy,
         ptr<Alien>           alien) :
        AlienProxy::Impl(proxy,
                         alien)
    { }
};

LocalAlienProxy::LocalAlienProxy(ptr<Alien> alien) :
    AlienProxy(make_ptr<Impl>(make_non_owning_ptr<LocalAlienProxy>(this), alien))
{ }

ptr<AlienProxy> LocalAlienProxy::clone() const
{
    return make_ptr<LocalAlienProxy>(priv(m_impl)->alienClone());
}

struct RemoteAlienProxy::Impl : public AlienProxy::Impl
{
    Impl(ptr<RemoteAlienProxy> proxy,
         ptr<Server>           server,
         int                   playerId,
         ptr<Alien>            alien) :
        AlienProxy::Impl(proxy,
                         alien),
        server(server),
        playerId(playerId)
    { }

    operation::Status checkStatus(operation::Status status,
                                  const string     &d=string())
    {
        switch (status)
        {
            case operation::Failure:
            {
                stringstream ss;
                ss <<
                    "Erreur de communication! Une opération a échouée pour le joueur " <<
                    playerId << ".";
                cerr << ss.str() << endl;

                return operation::Failure;
            }
            case operation::Timeout:
            {
                stringstream ss;
                ss << "Timeout from player : " << playerId;
                cerr << ss.str() << endl;

                return operation::Timeout;
            }
            case operation::Success:
            {
                if (!d.empty() && (d != data))
                {
                    stringstream ss;
                    ss << "Erreur de communication : mauvaises données, reçues : " << d <<
                        " attendues : " << data << ".";
                    cerr << ss.str() << endl;

                    return operation::Failure;
                }
                break;
            }
        }

        return operation::Success;
    }

    int checkCommandReply(const std::string &cmd,
                          const std::string &d)
    {
        stringstream ss(d);
        string       c;
        int          id;
        int          val;
        ss >> c >> id >> val;
        if (c != cmd)
        {
            stringstream ss;
            ss << "Erreur de communication : mauvaise commande envoyée par le client,";
            ss << " reçue : " << c << " attendue : " << cmd << "." << endl;
            cerr << ss.str() << endl;

            return -1;
        }
        if (id != alien->id())
        {
            stringstream ss;
            ss << "Erreur de communication : mauvais id d'alien envoyé par le client,";
            ss << " reçu : " << id << " attendu : " << alien->id() << "." << endl;
            cerr << ss.str() << endl;

            return -1;
        }

        return val;
    }

    /* move */
    virtual void queryMove()
    {
        stringstream ss;
        ss << Protocol::moveCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::queryMoveSent);
        server->write(playerId, data);
    }

    void queryMoveSent(operation::Status status,
                       const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::queryMoveSent);

        switch (checkStatus(status, d))
        {
            case operation::Failure: proxy->moveDecided(operation::Failure); return;

            case operation::Timeout: proxy->moveDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        server->dataRead[playerId].connect(this,
                                           &RemoteAlienProxy::Impl::queryMoveReceived);
        server->read(playerId);
    }

    void queryMoveReceived(operation::Status status,
                           const string     &d)
    {
        server->dataRead[playerId].disconnect(this,
                                              &RemoteAlienProxy::Impl::queryMoveReceived);

        switch (checkStatus(status))
        {
            case operation::Failure: proxy->moveDecided(operation::Failure); return;

            case operation::Timeout: proxy->moveDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        int ret = checkCommandReply(Protocol::moveCommand(), d);
        if (ret < 0)
        {
            proxy->moveDecided(operation::Failure);
        }
        else
        {
            Alien::Move move = static_cast<Alien::Move>(ret);
            proxy->moveDecided(operation::Success, move);
        }
    }

    /* eat */
    virtual void queryEat()
    {
        stringstream ss;
        ss << Protocol::eatCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::queryEatSent);
        server->write(playerId, data);
    }

    void queryEatSent(operation::Status status,
                      const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::queryEatSent);

        switch (checkStatus(status, d))
        {
            case operation::Failure: proxy->eatDecided(operation::Failure); return;

            case operation::Timeout: proxy->eatDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        server->dataRead[playerId].connect(this,
                                           &RemoteAlienProxy::Impl::queryEatReceived);
        server->read(playerId);
    }

    void queryEatReceived(operation::Status status,
                          const string     &d)
    {
        server->dataRead[playerId].disconnect(this,
                                              &RemoteAlienProxy::Impl::queryEatReceived);

        switch (checkStatus(status))
        {
            case operation::Failure: proxy->eatDecided(operation::Failure); return;

            case operation::Timeout: proxy->eatDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        int ret = checkCommandReply(Protocol::eatCommand(), d);
        if (ret < 0)
        {
            proxy->eatDecided(operation::Failure);
        }
        else
        {
            bool eat = static_cast<bool>(ret);
            proxy->eatDecided(operation::Success, eat);
        }
    }

    /* attack */
    virtual void queryAttack(Alien::Color   color,
                             Alien::Species species)
    {
        stringstream ss;
        ss << Protocol::attackCommand() << " " << alien->id() << " " << color << " " <<
            species;
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::queryAttackSent);
        server->write(playerId, data);
    }

    void queryAttackSent(operation::Status status,
                         const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::queryAttackSent);

        switch (checkStatus(status, d))
        {
            case operation::Failure: proxy->attackDecided(operation::Failure); return;

            case operation::Timeout: proxy->attackDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        server->dataRead[playerId].connect(this,
                                           &RemoteAlienProxy::Impl::queryAttackReceived);
        server->read(playerId);
    }

    void queryAttackReceived(operation::Status status,
                             const string     &d)
    {
        server->dataRead[playerId].disconnect(this,
                                              &RemoteAlienProxy::Impl::queryAttackReceived);

        switch (checkStatus(status))
        {
            case operation::Failure: proxy->attackDecided(operation::Failure); return;

            case operation::Timeout: proxy->attackDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        int ret = checkCommandReply(Protocol::attackCommand(), d);
        if (ret < 0)
        {
            proxy->attackDecided(operation::Failure); return;
        }
        else
        {
            Alien::Attack attack = static_cast<Alien::Attack>(ret);
            proxy->attackDecided(operation::Success, attack);
        }
    }

    /* color */
    virtual void queryColor()
    {
        stringstream ss;
        ss << Protocol::colorCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::queryColorSent);
        server->write(playerId, data);
    }

    void queryColorSent(operation::Status status,
                        const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::queryColorSent);

        switch (checkStatus(status, d))
        {
            case operation::Failure: proxy->colorDecided(operation::Failure); return;

            case operation::Timeout: proxy->colorDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        server->dataRead[playerId].connect(this,
                                           &RemoteAlienProxy::Impl::queryColorReceived);
        server->read(playerId);
    }

    void queryColorReceived(operation::Status status,
                            const string     &d)
    {
        server->dataRead[playerId].disconnect(this,
                                              &RemoteAlienProxy::Impl::queryColorReceived);

        switch (checkStatus(status))
        {
            case operation::Failure: proxy->colorDecided(operation::Failure); return;

            case operation::Timeout: proxy->colorDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        int ret = checkCommandReply(Protocol::colorCommand(), d);
        if (ret < 0)
        {
            proxy->colorDecided(operation::Failure); return;
        }
        else
        {
            Alien::Color color = static_cast<Alien::Color>(ret);
            proxy->colorDecided(operation::Success, color);
        }
    }

    /* species */
    virtual void querySpecies()
    {
        stringstream ss;
        ss << Protocol::speciesCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::querySpeciesSent);
        server->write(playerId, data);
    }

    void querySpeciesSent(operation::Status status,
                          const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::querySpeciesSent);

        switch (checkStatus(status, d))
        {
            case operation::Failure: proxy->speciesDecided(operation::Failure); return;

            case operation::Timeout: proxy->speciesDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        server->dataRead[playerId].connect(this,
                                           &RemoteAlienProxy::Impl::querySpeciesReceived);
        server->read(playerId);
    }

    void querySpeciesReceived(operation::Status status,
                              const string     &d)
    {
        server->dataRead[playerId].disconnect(this,
                                              &RemoteAlienProxy::Impl::querySpeciesReceived);

        switch (checkStatus(status))
        {
            case operation::Failure: proxy->speciesDecided(operation::Failure); return;

            case operation::Timeout: proxy->speciesDecided(operation::Timeout); return;

            case operation::Success: break;
        }

        int ret = checkCommandReply(Protocol::speciesCommand(), d);
        if (ret < 0)
        {
            proxy->speciesDecided(operation::Failure); return;
        }
        else
        {
            Alien::Species species = static_cast<Alien::Species>(ret);
            proxy->speciesDecided(operation::Success, species);
        }
    }

    /* turn */
    virtual void infoTurn(int turn)
    {
        stringstream ss;
        ss << Protocol::turnCommand() << " " << alien->id() << " " << turn;
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoTurnSent);
        server->write(playerId, data);
    }

    void infoTurnSent(operation::Status status,
                      const string     &d)
    {
        if (checkStatus(status, d) == operation::Success)
        {
            server->dataWritten[playerId].disconnect(this,
                                                     &RemoteAlienProxy::Impl::infoTurnSent);

            proxy->turnDispatched();
        }
    }

    /* spawn */
    virtual void infoSpawn(int babyId)
    {
        stringstream ss;
        ss << Protocol::spawnCommand() << " " << alien->id() << " " << babyId;
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoSpawnSent);
        server->write(playerId, data);
    }

    void infoSpawnSent(operation::Status status,
                       const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoSpawnSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->spawnDispatched();
        }
    }

    /* status */
    virtual void infoStatus(int x,
                            int y,
                            int width,
                            int height,
                            int energy)
    {
        stringstream ss;
        ss << Protocol::statusCommand() << " " << alien->id() << " " << x << " " << y <<
            " " << width << " " << height << " " << energy;
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoStatusSent);
        server->write(playerId, data);
    }

    void infoStatusSent(operation::Status status,
                        const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoStatusSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->statusDispatched();
        }
    }

    /* neighboor */
    virtual void infoNeighboor(int            x,
                               int            y,
                               Alien::Color   color,
                               Alien::Species species,
                               bool           sleeping,
                               bool           mating,
                               bool           eating)
    {
        stringstream ss;
        ss << Protocol::neighboorCommand() << " " << alien->id() << " "
           << x << " " << y << " " << color << " " << species << " "
           << static_cast<int>(sleeping) << " "
           << static_cast<int>(mating) << " "
           << static_cast<int>(eating);
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoNeighboorSent);
        server->write(playerId, data);
    }

    void infoNeighboorSent(operation::Status status,
                           const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoNeighboorSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->neighboorDispatched();
        }
    }

    /* sleep */
    virtual void infoSleep()
    {
        stringstream ss;
        ss << Protocol::sleepCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoSleepSent);
        server->write(playerId, data);
    }

    void infoSleepSent(operation::Status status,
                       const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoSleepSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->sleepDispatched();
        }
    }

    /* wakeup */
    virtual void infoWakeup()
    {
        stringstream ss;
        ss << Protocol::wakeupCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoWakeupSent);
        server->write(playerId, data);
    }

    void infoWakeupSent(operation::Status status,
                        const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoWakeupSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->wakeupDispatched();
        }
    }

    /* mate */
    virtual void infoMate()
    {
        stringstream ss;
        ss << Protocol::mateCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoMateSent);
        server->write(playerId, data);
    }

    void infoMateSent(operation::Status status,
                      const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoMateSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->mateDispatched();
        }
    }

    /* win */
    virtual void infoWin()
    {
        stringstream ss;
        ss << Protocol::winCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this, &RemoteAlienProxy::Impl::infoWinSent);
        server->write(playerId, data);
    }

    void infoWinSent(operation::Status status,
                     const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoWinSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->winDispatched();
        }
    }

    /* lose */
    virtual void infoLose()
    {
        stringstream ss;
        ss << Protocol::loseCommand() << " " << alien->id();
        data = ss.str();
        server->dataWritten[playerId].connect(this,
                                              &RemoteAlienProxy::Impl::infoLoseSent);
        server->write(playerId, data);
    }

    void infoLoseSent(operation::Status status,
                      const string     &d)
    {
        server->dataWritten[playerId].disconnect(this,
                                                 &RemoteAlienProxy::Impl::infoLoseSent);

        if (checkStatus(status, d) == operation::Success)
        {
            proxy->loseDispatched();
        }
    }

    ptr<Server> server;
    int         playerId;
    string      data;
};

RemoteAlienProxy::RemoteAlienProxy(ptr<Server> server,
                                   int         playerId,
                                   ptr<Alien>  alien) :
    AlienProxy(make_ptr<Impl>(make_non_owning_ptr<RemoteAlienProxy>(this), server,
                              playerId, alien))
{ }

ptr<AlienProxy> RemoteAlienProxy::clone() const
{
    return make_ptr<RemoteAlienProxy>(
        static_cast<RemoteAlienProxy::Impl *>(priv(m_impl))->server,
        static_cast<RemoteAlienProxy::Impl *>(priv(m_impl))->playerId,
        priv(m_impl)->alienClone());
}
