#ifndef ALIEN_PROXY_H
#define ALIEN_PROXY_H

#include "alien/alien.h"
#include "src/signal.h"
#include "src/common.h"
#include "src/ptr.h"

// la classe est uniquement accessible depuis Simulation
// permet de représenter un alien local ou remote de manière transparente
class AlienProxy
{
    public:
        struct Impl;

        virtual ~AlienProxy();

        ptr<Alien> alien() const;

        virtual ptr<AlienProxy> clone() const = 0;

        // appelé pour savoir le déplacement que l'agent veut entreprendre
        Signal<operation::Status, Alien::Move> moveDecided;
        void queryMove(int turn);

        // appelé pour savoir si l'agent veut manger la nourriture sur la case
        // sur laquelle il se trouve
        Signal<operation::Status, bool> eatDecided;
        void queryEat(int turn);

        // appelé pour savoir comment l'agent veut attaquer un autre agent
        Signal<operation::Status, Alien::Attack> attackDecided;
        void queryAttack(int            turn,
                         Alien::Color   alienColor,
                         Alien::Species alienSpecies);

        // appelé pour savoir la couleur d'un agent
        Signal<operation::Status, Alien::Color> colorDecided;
        void queryColor(int turn);

        // appelé pour savoir l'espèce que l'agent veut fournir aux autres
        Signal<operation::Status, Alien::Species> speciesDecided;
        void querySpecies(int turn);

        // appelé pour donner le tour courant
        Signal<> turnDispatched;
        void infoTurn(int turn) const;

        // appelé pour prévenir qu'un alien va naitre
        Signal<> spawnDispatched;
        void infoSpawn(int turn,
                       int babyId) const;

        // appelé pour donner le statut de cet agent (position, bornes, energie)
        Signal<> statusDispatched;
        void infoStatus(int turn,
                        int x,
                        int y,
                        int width,
                        int height,
                        int energy);

        // appelé pour chaque voisin que l'agent voit
        Signal<> neighboorDispatched;
        void infoNeighboor(int            turn,
                           int            x,
                           int            y,
                           Alien::Color   color,
                           Alien::Species species,
                           bool           sleeping,
                           bool           mating,
                           bool           eating);

        // appelé pour chaque nourriture que l'agent voit
        Signal<> foodDispatched;
        void infoFood(int turn,
                      int x,
                      int y);

        // appelé lorsque l'agent doit dormir (plus d'énergie)
        Signal<> sleepDispatched;
        void infoSleep(int turn);

        // appelé lorsque l'agent se réveille
        Signal<> wakeupDispatched;
        void infoWakeup(int turn);

        // appelé lorsque l'agent est en train de se reproduire
        Signal<> mateDispatched;
        void infoMate(int turn);

        // appelé lorsque l'agent a gagné un combat
        Signal<> winDispatched;
        void infoWin(int turn);

        // appelé lorsque l'agent a perdu un combat et va mourir
        Signal<> loseDispatched;
        void infoLose(int turn);

        // appelé lorsque l'alien fait une action qui coute ou redonne de l'énergie
        Signal<> energyAdded;
        void addEnergy(int turn,
                       int delta);

        Signal<> energyRemoved;
        void removeEnergy(int turn,
                          int delta);

    protected:
        AlienProxy(ptr<BasePrivImpl> impl);
        ptr<BasePrivImpl> m_impl;

    private:
        AlienProxy(const AlienProxy &) { }
        void operator=(const AlienProxy &) { }
};

class LocalAlienProxy : public AlienProxy
{
    public:
        LocalAlienProxy(ptr<Alien> alien);
        virtual ptr<AlienProxy> clone() const;

    protected:
        struct Impl;
};

class Server;
class RemoteAlienProxy : public AlienProxy
{
    public:
        RemoteAlienProxy(ptr<Server> server,
                         int         playerId,
                         ptr<Alien>  alien);
        virtual ptr<AlienProxy> clone() const;

    protected:
        struct Impl;
};
#endif // ALIEN_PROXY_H
