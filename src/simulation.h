#ifndef SIMULATION_H
#define SIMULATION_H

#include <QObject>
#include <stdexcept>
#include <set>
#include <map>
#include "src/board.h"
#include "src/gui.h"
#include "src/ptr.h"
#include "src/common.h"
#include "src/signal.h"
#include "server/server.h"

struct SimulationException : public std::runtime_error
{
    SimulationException(const std::string &what) :
        std::runtime_error(what) { }
};

class Simulation
{
    public:
        struct Impl;

        Simulation(int width,
                   int height);

        int width() const;
        int height() const;
        const std::vector< ptr<Alien> > aliens() const;

        int numPlayers() const;

        // ajoute un alien dans la map et le place à une position aléatoire
        void addLocalAlien(ptr<Alien> alien);
        void addRemoteAlien(ptr<Server> server,
                            int         playerId,
                            ptr<Alien>  alien);

        // commence la simulation
        void start();
        bool started();

        void stop();
        bool stopped();

        bool over();
        int turn();
        void update();
        bool updating() const;

        void showMessage(const std::string &);
        Signal<std::string> msgPending;
        Signal<>            msgAcked;

        enum Constants
        {
            MaxNumSpecies         = 6,
            PercentageOfFood      = 5,
            FoodRespawnTurns      = 5,
            SleepingTurnsIfTired  = 6,
            SleepingTurnsIfMating = 4,
            SleepingTurnsIfEating = 2,
            MaxEnergy             = 50,
            EatingBonus           = MaxEnergy / 2,
            MovePenalty           = 3,
            NoMovePenalty         = 1,
            MaxTurns              = 200,
        };

    private:
        ptr<BasePrivImpl> m_impl;
};
#endif // SIMULATION_H
