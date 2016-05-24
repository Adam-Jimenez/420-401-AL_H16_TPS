#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <map>
#include <typeinfo>
#include "src/signal.h"
#include "src/common.h"
#include "src/simulation.h"
#include "src/updater.h"
#include "alien/aliens.h"
#include "alien/proxy.h"

#include <iostream>

using namespace std;

int mod(int a,
        int b)
{
    int r = a % b;

    return r < 0 ? r + b : r;
}

namespace
{
    namespace updateSimStatus
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation::Impl> impl);
            void run();
            void step();

            ptr<Simulation::Impl>     impl;
            Signal<operation::Status> done;
        };
    }

    namespace disqualify
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation::Impl> impl);
            void run();
            void update();
            void finish(operation::Status);

            ptr<updateSimStatus::Operation> op;
            ptr<Simulation::Impl>           impl;
            Signal<operation::Status>       done;
        };
    }

    namespace updateAlien
    {
        struct Operation : public operation::Immediate<Operation, bool>
        {
            Operation(ptr<Simulation::Impl> impl);
            bool checkStatus(operation::Status status);
            void run();
            void checkSleeping();
            void wakeUpAfterSleeping();
            void wakeUpAfterSleepingPost();
            void checkMating();
            void wakeUpAfterMating();
            void wakeUpAfterMatingPost();
            void spawnBaby();
            void spawnBabyPost();
            void spawn2Babies();
            void spawn2BabiesFirst();
            void spawn2BabiesFirstPost();
            void spawn2BabiesSecond();
            void spawn2BabiesSecondPost();
            void spawnPost();
            void checkEating();
            void wakeUpAfterEating();
            void wakeUpAfterEatingPost();
            void sendStatus();
            void sendStatusPost();
            void sendNeighboorLoopPre();
            void sendNeighboorLoop();
            void sendNeighboorLoopPost();
            void sendNeighboor();
            void sendNeighboorPost();
            void sendFoodLoopPre();
            void sendFoodLoop();
            void sendFoodLoopPost();
            void sendFood();
            void sendFoodPost();
            void recvMove();
            void recvMovePost(operation::Status status,
                              Alien::Move       move);
            void dontMove();
            void dontMovePost();
            void checkFoodAfterMove();
            void recvEat();
            void recvEatPost(operation::Status status,
                             bool              eat);
            void checkAlienAfterMove();
            void sendMate();
            void sendMateFirst();
            void sendMateSecond();
            void sendMatePost();
            void sendWin();
            void sendWinPost();
            void sendOtherLose();
            void sendOtherLosePost();
            void sendLose();
            void sendLosePost();
            void sendOtherWin();
            void sendOtherWinPost();
            void recvAttack();
            void recvAttackFirst();
            void recvAttackFirstPost(operation::Status status,
                                     Alien::Attack     attack);
            void recvAttackSecond();
            void recvAttackSecondPost(operation::Status status,
                                      Alien::Attack     attack);
            void checkFight();
            void finallyMove();
            void finallyMovePost();

            ptr<Alien>      alien;
            ptr<AlienProxy> proxy;
            // voisins
            vector< ptr<Alien> >           neighboors;
            vector< ptr<Alien> >::iterator neighboorsIt;
            // bouffe
            vector< pair<int, int> >           foods;
            vector< pair<int, int> >::iterator foodsIt;
            // combat
            ptr<Alien>      oAlien;
            ptr<AlienProxy> oProxy;
            Alien::Attack   attackAlien;
            Alien::Attack   attackOtherAlien;
            // case
            int                             x, y; // position de l'alien avant move
            int                             nx, ny; // position de l'alien après move
            ptr<Simulation::Impl>           impl;
            Signal<operation::Status, bool> done;
        };
    }

    namespace beginUpdate
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation::Impl> impl);

            void run();
            void updateColorSpeciesLoopPre();
            void updateColorSpeciesLoop();
            void updateColorSpeciesLoopPost();
            void updateColorSpecies();
            void updateTurnPre();
            void updateTurn();
            void updateTurnPost();
            void updateColorPre();
            void updateColor(operation::Status status,
                             Alien::Color);
            void updateDisqualify();
            void updateDisqualifyPost(operation::Status status);
            void updateColorPost();
            void updateSpeciesPre();
            void updateSpecies(operation::Status status,
                               Alien::Species);
            void updateSpeciesPost();
            void updateAlienStatus();
            void updateColorSpeciesPost();
            int index;
            int count;

            ptr<disqualify::Operation> dis_op;
            ptr<Simulation::Impl>      impl;
            Signal<operation::Status>  done;
        };
    }

    namespace updateTurn
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation::Impl> impl);
            void run();
            void updateAlienLoopPre();
            void updateAlienLoop();
            void updateAlien();
            void updateAlienCheck(operation::Status status,
                                  bool);
            void updateDisqualify();
            void updateDisqualifyPost(operation::Status status);
            void updateAlienPost();
            void updateAlienLoopPost();

            int                         count;
            ptr<updateAlien::Operation> upd_op;
            ptr<disqualify::Operation>  dis_op;
            ptr<Simulation::Impl>       impl;
            Signal<operation::Status>   done;
        };
    }

    namespace updateTick
    {
        struct Operation : public operation::Immediate<Operation>
        {
            Operation(ptr<Simulation::Impl> impl);
            void run();
            void updateAlienLoopPre();
            void updateAlienLoop();
            void updateAlien();
            void updateAlienCheck(operation::Status status,
                                  bool              ret);
            void updateDisqualify();
            void updateDisqualifyPost(operation::Status status);
            void updateAlienPost(bool ret);

            int                         count;
            ptr<updateAlien::Operation> upd_op;
            ptr<disqualify::Operation>  dis_op;
            ptr<Simulation::Impl>       impl;
            Signal<operation::Status>   done;
        };
    }
}

struct Simulation::Impl : public BasePrivImpl
{
    Impl(ptr<Simulation> sim,
         int             width,
         int             height) :
        sim(sim),
        tick(-1),
        turn(0),
        started(false),
        over(false),
        gui(make_ptr<Gui>(width, height)),
        board(make_ptr<Board>(width, height, gui)),
        updater(sim, gui),
        updating(false)
    {
        sim->msgPending.connect(this, &Simulation::Impl::showMessage);
    }

    void update();
    void beginUpdate();
    void beginUpdatePost(operation::Status);
    ptr<beginUpdate::Operation> beg_op;
    void doUpdate();
    void endUpdate();
    void doTurn();
    void doTurnPost(operation::Status);
    ptr<updateTurn::Operation> upd_op;
    void updateStats();
    void updateStatsPost(operation::Status);
    ptr<updateSimStatus::Operation> stat_op;
    void doTick();
    void doTickPost(operation::Status);
    ptr<updateTick::Operation> upt_op;
    void showMessage(const string &message);

    ptr<Simulation> sim;

    int        tick;
    int        turn;
    bool       started;
    bool       over;
    ptr<Gui>   gui;
    ptr<Board> board;

    map< ptr<Alien>, ptr<AlienProxy> > proxys;
    vector< ptr<Alien> >               currentAlienPerm;
    set< ptr<Alien> >                  matingAliens;
    set<int>                           species;
    Updater                            updater;
    bool                               updating;
};

static Simulation::Impl * priv(const ptr<BasePrivImpl> &p)
{
    return static_cast<Simulation::Impl *>(p.get());
}

namespace
{
    namespace updateSimStatus
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation>(done),
            impl(impl)
        { }

        void Operation::run()
        {
            int         numCells = impl->board->width() * impl->board->height();
            vector<int> alienCounts(Simulation::MaxNumSpecies);
            for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
            {
                Alien::Species sp = static_cast<Alien::Species>(i);
                alienCounts[i] = impl->board->countAliens(sp);
            }

            // 1. plus aucun adversaire
            int speciesWinner     = -1;
            int countSpeciesAlive = 0;
            for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
            {
                if (alienCounts[i] != 0)
                {
                    countSpeciesAlive++;
                    speciesWinner = i;
                }
            }
            if (countSpeciesAlive != 1)
            {
                // plus que 1 équipe en vie !
                speciesWinner = -1;
            }

            // 2. occupe toute la carte
            if (speciesWinner < 0)
            {
                for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
                {
                    if (alienCounts[i] / double(numCells) > 0.5)
                    {
                        // cet équipe a gagné
                        speciesWinner = i;
                        break;
                    }
                }
            }

            // 3. plus de tours à jouer !
            if (speciesWinner < 0)
            {
                if (impl->turn >= Simulation::MaxTurns)
                {
                    int maxCount = -1;
                    for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
                    {
                        if (alienCounts[i] > maxCount)
                        {
                            // cet équipe a gagné
                            speciesWinner = i;
                            maxCount      = alienCounts[i];
                        }
                    }
                }
            }

            if (speciesWinner >= 0)
            {
                ostringstream ss;
                ss << "La simulation s'est terminée par la victoire des "
                   << Alien::speciesString(static_cast<Alien::Species>(speciesWinner))
                   << "!";

                impl->sim->msgAcked.connect(this, &updateSimStatus::Operation::step);
                impl->sim->msgPending(ss.str());
            }
            else
            {
                success();
            }
        }

        void Operation::step()
        {
            impl->sim->msgAcked.disconnect(this, &updateSimStatus::Operation::step);
            impl->over = true;
            impl->gui->setUpdateType(Gui::NONE);
            success();
        }
    }
}

namespace
{
    namespace disqualify
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation>(done),
            impl(impl)
        { }

        void Operation::run()
        {
            ptr<Alien>     alien          = impl->currentAlienPerm[impl->tick];
            Alien::Species spDisqualified = alien->realSpecies();

            size_t nbs = impl->currentAlienPerm.size();
            // l'alien a fail ! on élimine l'équipe !!
            for (size_t i = 0; i < nbs; ++i)
            {
                if (impl->currentAlienPerm[i]->realSpecies() == spDisqualified)
                {
                    impl->board->removeAlien(impl->currentAlienPerm[i]);
                }
            }

            stringstream ss;
            ss << "L'alien d'id " << alien->id() << " n'a pas donné " <<
                "de réponse valide ou dans le temps imparti à l'oracle, ou une erreur s'est produite. Son espèce "
               << Alien::speciesString(spDisqualified) << " a donc été disqualifiée.";

            impl->sim->msgAcked.connect(this, &disqualify::Operation::update);
            impl->sim->msgPending(ss.str());
        }

        void Operation::update()
        {
            impl->sim->msgAcked.disconnect(this, &disqualify::Operation::update);
            impl->updateStats();

            op = make_ptr<updateSimStatus::Operation>(impl);
            op->done.connect(this, &disqualify::Operation::finish);
            op->exec();
        }

        void Operation::finish(operation::Status)
        {
            op->done.disconnect(this, &disqualify::Operation::finish);
            success();
        }
    }
}

namespace
{
    namespace updateAlien
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation, bool>(done),
            impl(impl)
        { }

        bool Operation::checkStatus(operation::Status status)
        {
            switch (status)
            {
                case operation::Failure:
                {
                    fail();

                    return false;
                }
                case operation::Timeout:
                {
                    timeout();

                    return false;
                }
                case operation::Success:
                    break;
            }

            return true;
        }

        void Operation::run()
        {
            bool canRun = false;
            alien = impl->currentAlienPerm[impl->tick];
            proxy = impl->proxys[alien];
            if (!alien.empty())
            {
                // trouve la position de l'alien !
                pair<int, int> pos = (*impl->board)[alien];
                x = pos.first;
                y = pos.second;
                if ((x >= 0) && (y >= 0))
                {
                    // devrait jamais arriver
                    if ((x >= impl->board->width()) || (y >= impl->board->height()))
                    {
                        throw SimulationException("Coordonnées d'un alien incohérentes!");
                        // on arrivera jamais la
                    }

                    canRun = true;
                }
                else
                {
                    // peut arriver si l'alien s'est fait tué !
                }
            }
            if (canRun)
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "-----" << endl;
                    oss << "Mis à jour de l'alien " << alien->id()
                        << " d'énergie " << alien->energy();
                    impl->gui->appendDebug(oss.str());
                }

                checkSleeping();
            }
            else
            {
                success(false);
            }
        }

        void Operation::checkSleeping()
        {
            // est-ce que l'alien dort ?
            if (alien->sleeping())
            {
                // compte le nombre de tour écoulé depuis qu'il dort !
                int dturn = impl->turn - alien->sleepingTurn();
                if (dturn >= Simulation::SleepingTurnsIfTired)
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien se réveille";
                        impl->gui->appendDebug(oss.str());
                    }
                    wakeUpAfterSleeping();
                }
                else
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien doit encore dormir "
                            << (Simulation::SleepingTurnsIfTired - dturn) << " tours";
                        impl->gui->appendDebug(oss.str());
                    }
                    // rien à faire ! l'alien dort
                    success(true);
                }
            }
            else
            {
                checkMating();
            }
        }

        void Operation::wakeUpAfterSleeping()
        {
            // réveille l'alien, il a assez dormi
            proxy->wakeupDispatched.connect(this,
                                            &updateAlien::Operation::wakeUpAfterSleepingPost);
            proxy->infoWakeup(impl->turn);
        }

        void Operation::wakeUpAfterSleepingPost()
        {
            // réveille l'alien, il a assez dormi
            proxy->wakeupDispatched.disconnect(this,
                                               &updateAlien::Operation::wakeUpAfterSleepingPost);

            checkMating();
        }

        void Operation::checkMating()
        {
            // est-ce qu'il s'accouple
            if (alien->mating())
            {
                int dturn = impl->turn - alien->matingTurn();
                if (dturn >= Simulation::SleepingTurnsIfMating)
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien vient de finir de s'accoupler";
                        impl->gui->appendDebug(oss.str());
                    }
                    wakeUpAfterMating();

                }
                else
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien doit encore se reposer "
                            << (Simulation::SleepingTurnsIfMating - dturn) << " tours";
                        impl->gui->appendDebug(oss.str());
                    }
                    // rien à faire ! l'alien dort
                    success(true);
                }
            }
            else
            {
                checkEating();
            }
        }

        void Operation::wakeUpAfterMating()
        {
            // réveille l'alien, il a assez dormi
            proxy->wakeupDispatched.connect(this,
                                            &updateAlien::Operation::wakeUpAfterMatingPost);
            proxy->infoWakeup(impl->turn);
        }

        void Operation::wakeUpAfterMatingPost()
        {
            proxy->wakeupDispatched.disconnect(this,
                                               &updateAlien::Operation::wakeUpAfterMatingPost);

            if (impl->matingAliens.find(alien) != impl->matingAliens.end())
            {
                // 0 bébé 5%
                // 1 bébé 85%
                // 2 bébés 10%
                int r = rand() % 100;
                if (r < 5)
                {
                    // 0 bébé : pas de chances !
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "Malheuresement, aucun bébé n'a été créé!";
                        impl->gui->appendDebug(oss.str());
                    }
                    spawnPost();
                }
                else if (r < 5 + 85)
                {
                    // 1 bébé au hasard !
                    spawnBaby();
                }
                else
                {
                    // 2 bébés au hasard !
                    spawn2Babies();
                }
            }
            else
            {
                // l'autre alien du couple fera les bébés
                checkEating();
            }
        }

        void Operation::spawnBaby()
        {
            if (impl->board->full())
            {
                throw SimulationException("La carte est pleine!");
                // on arrivera jamais la
            }

            pair<int, int>  p         = impl->board->randomEmptySpot();
            ptr<AlienProxy> babyProxy = proxy->clone();
            ptr<Alien>      baby      = babyProxy->alien();
            impl->proxys[baby] = babyProxy;
            impl->board->addAlien(baby, p.first, p.second);

            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Un bébé n'a été créé à la case (" << p.first
                    << "," << p.second << ")";
                impl->gui->appendDebug(oss.str());
            }

            proxy->spawnDispatched.connect(this, &updateAlien::Operation::spawnBabyPost);
            proxy->infoSpawn(impl->turn, baby->id());
        }

        void Operation::spawnBabyPost()
        {
            proxy->spawnDispatched.disconnect(this,
                                              &updateAlien::Operation::spawnBabyPost);

            spawnPost();
        }

        void Operation::spawn2Babies()
        {
            if (impl->board->full())
            {
                throw SimulationException("La carte est pleine!");
                // on arrivera jamais la
            }

            pair<int, int>  p         = impl->board->randomEmptySpot();
            ptr<AlienProxy> babyProxy = proxy->clone();
            ptr<Alien>      baby      = babyProxy->alien();
            impl->proxys[baby] = babyProxy;
            impl->board->addAlien(baby, p.first, p.second);

            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Un premier bébé "
                    << " a été créé à la case (" << p.first
                    << "," << p.second << ")";
                impl->gui->appendDebug(oss.str());
            }

            proxy->spawnDispatched.connect(this,
                                           &updateAlien::Operation::spawn2BabiesFirst);
            proxy->infoSpawn(impl->turn, baby->id());
        }

        void Operation::spawn2BabiesFirst()
        {
            proxy->spawnDispatched.disconnect(this,
                                              &updateAlien::Operation::spawn2BabiesFirst);

            spawn2BabiesFirstPost();
        }

        void Operation::spawn2BabiesFirstPost()
        {
            if (impl->board->full())
            {
                throw SimulationException("La carte est pleine!");
                // on arrivera jamais la
            }

            pair<int, int>  p         = impl->board->randomEmptySpot();
            ptr<AlienProxy> babyProxy = proxy->clone();
            ptr<Alien>      baby      = babyProxy->alien();
            impl->proxys[baby] = babyProxy;
            impl->board->addAlien(baby, p.first, p.second);

            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Un second bébé "
                    << " a été créé à la case (" << p.first
                    << "," << p.second << ")";
                impl->gui->appendDebug(oss.str());
            }

            proxy->spawnDispatched.connect(this,
                                           &updateAlien::Operation::spawn2BabiesSecond);
            proxy->infoSpawn(impl->turn, baby->id());
        }

        void Operation::spawn2BabiesSecond()
        {
            proxy->spawnDispatched.disconnect(this,
                                              &updateAlien::Operation::spawn2BabiesSecond);

            spawn2BabiesSecondPost();
        }

        void Operation::spawn2BabiesSecondPost()
        {
            spawnPost();
        }

        void Operation::spawnPost()
        {
            impl->matingAliens.erase(alien);
            checkEating();
        }

        void Operation::checkEating()
        {
            if (alien->eating())
            {
                // compte le nombre de tour écoulé depuis qu'il dort !
                int dturn = impl->turn - alien->eatingTurn();
                if (dturn >= Simulation::SleepingTurnsIfEating)
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien vient de finir de manger";
                        impl->gui->appendDebug(oss.str());
                    }
                    wakeUpAfterEating();
                }
                else
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "L'alien doit encore digérer "
                            << (Simulation::SleepingTurnsIfEating - dturn) << " tours";
                        impl->gui->appendDebug(oss.str());
                    }
                    // rien à faire, l'alien digère
                    success(true);
                }
            }
            else
            {
                sendStatus();
            }
        }

        void Operation::wakeUpAfterEating()
        {
            // réveille l'alien, il a assez dormi
            proxy->wakeupDispatched.connect(this,
                                            &updateAlien::Operation::wakeUpAfterEatingPost);
            proxy->infoWakeup(impl->turn);
        }

        void Operation::wakeUpAfterEatingPost()
        {
            proxy->wakeupDispatched.disconnect(this,
                                               &updateAlien::Operation::wakeUpAfterEatingPost);
            sendStatus();
        }

        void Operation::sendStatus()
        {
            proxy->statusDispatched.connect(this,
                                            &updateAlien::Operation::sendStatusPost);
            // envoit les coordonnées à l'alien !
            proxy->infoStatus(impl->turn, x, y,
                              impl->board->width(), impl->board->height(),
                              alien->energy());
        }

        void Operation::sendStatusPost()
        {
            proxy->statusDispatched.disconnect(this,
                                               &updateAlien::Operation::sendStatusPost);

            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Position de l'alien (" << x << "," << y << ")"
                    << " énergie : " << alien->energy();
                impl->gui->appendDebug(oss.str());
            }

            sendNeighboorLoopPre();
        }

        void Operation::sendNeighboorLoopPre()
        {
            neighboors   = impl->board->neighboors(alien);
            neighboorsIt = neighboors.begin();
            sendNeighboorLoop();
        }

        void Operation::sendNeighboorLoop()
        {
            if (neighboorsIt != neighboors.end())
            {
                sendNeighboor();

            }
            else
            {
                sendNeighboorLoopPost();
            }
        }

        void Operation::sendNeighboorLoopPost()
        {
            sendFoodLoopPre();
        }

        void Operation::sendNeighboor()
        {
            proxy->neighboorDispatched.connect(this,
                                               &updateAlien::Operation::sendNeighboorPost);

            ptr<Alien>     nalien = *neighboorsIt;
            pair<int, int> pos    = (*impl->board)[nalien];
            proxy->infoNeighboor(impl->turn,
                                 pos.first, pos.second,
                                 nalien->color(),
                                 nalien->species(),
                                 nalien->sleeping(),
                                 nalien->mating(),
                                 nalien->eating());
        }

        void Operation::sendNeighboorPost()
        {
            proxy->neighboorDispatched.disconnect(this,
                                                  &updateAlien::Operation::sendNeighboorPost);

            if (impl->gui->debug())
            {
                ptr<Alien>     nalien = *neighboorsIt;
                pair<int, int> pos    = (*impl->board)[nalien];
                ostringstream  oss;
                oss << "Voit un voisin alien d'id " << nalien->id()
                    << " à la position (" << pos.first << "," << pos.second << ") "
                    << " de couleur " << nalien->color()
                    << " d'espèce " << nalien->species()
                    << " qui " << (nalien->sleeping() ? "dort" : "ne dort pas") << ", "
                    << (nalien->mating() ? "se reproduit" : "ne se reproduit pas") << ", "
                    << (nalien->eating() ? "mange" : "ne mange pas");
                impl->gui->appendDebug(oss.str());
            }

            ++neighboorsIt;
            sendNeighboorLoop();
        }

        void Operation::sendFoodLoopPre()
        {
            foods   = impl->board->foods(alien);
            foodsIt = foods.begin();
            sendFoodLoop();
        }

        void Operation::sendFoodLoop()
        {
            if (foodsIt != foods.end())
            {
                sendFood();

            }
            else
            {
                sendFoodLoopPost();
            }
        }

        void Operation::sendFoodLoopPost()
        {
            recvMove();
        }

        void Operation::sendFood()
        {
            proxy->foodDispatched.connect(this, &updateAlien::Operation::sendFoodPost);

            pair<int, int> food = *foodsIt;
            proxy->infoFood(impl->turn, food.first, food.second);
        }

        void Operation::sendFoodPost()
        {
            proxy->foodDispatched.disconnect(this, &updateAlien::Operation::sendFoodPost);

            if (impl->gui->debug())
            {
                pair<int, int> food = *foodsIt;
                ostringstream  oss;
                oss << "Voit de la nourriture "
                    << " à la position (" << food.first << "," << food.second << ")";
                impl->gui->appendDebug(oss.str());
            }

            ++foodsIt;
            sendFoodLoop();
        }

        void Operation::recvMove()
        {
            proxy->moveDecided.connect(this, &updateAlien::Operation::recvMovePost);
            proxy->queryMove(impl->turn);
        }

        void Operation::recvMovePost(operation::Status status,
                                     Alien::Move       move)
        {
            proxy->moveDecided.disconnect(this, &updateAlien::Operation::recvMovePost);

            if (checkStatus(status))
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "Choisit le mouvement " << Alien::moveString(move);
                    impl->gui->appendDebug(oss.str());
                }

                if (move == Alien::None)
                {
                    dontMove();
                }
                else
                {
                    switch (move)
                    {
                        case Alien::Up:
                            nx = x; ny = mod(y - 1, impl->board->height());
                            break;
                        case Alien::Down:
                            nx = x; ny = mod(y + 1, impl->board->height());
                            break;
                        case Alien::Left:
                            nx = mod(x - 1, impl->board->width()); ny = y;
                            break;
                        case Alien::Right:
                            nx = mod(x + 1, impl->board->width()); ny = y;
                            break;
                        case Alien::None:
                            break;
                    }
                    if ((nx >= impl->board->width()) || (y >= impl->board->height()))
                    {
                        stringstream ss;
                        ss << "BUGGG ! pas supposé de se produire ! " << alien.get() <<
                            " " << x << "  " <<  y << " " << move << endl;
                        throw SimulationException(ss.str());
                        // on arrivera jamais la
                    }
                    checkFoodAfterMove();
                }
            }
        }

        void Operation::dontMove()
        {
            // l'alien ne veut pas bouger, il perd 1 énergie !
            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Ne bouge pas et perd " << Simulation::NoMovePenalty << " énergie";
                impl->gui->appendDebug(oss.str());
            }

            proxy->energyRemoved.connect(this, &updateAlien::Operation::dontMovePost);
            proxy->removeEnergy(impl->turn, Simulation::NoMovePenalty);
        }

        void Operation::dontMovePost()
        {
            proxy->energyRemoved.disconnect(this, &updateAlien::Operation::dontMovePost);
            // l'alien a perdu son énergie, et dort ou ne fait plus rien
            if (alien->energy() == 0)
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "N'a plus d'énergie et doit dormir";
                    impl->gui->appendDebug(oss.str());
                }
            }
            success(true);
        }

        void Operation::checkFoodAfterMove()
        {
            // qu'est-ce qu'il y a sur la case ?
            if (impl->board->foodAt(nx, ny))
            {
                recvEat();
            }
            else
            {
                checkAlienAfterMove();
            }
        }

        void Operation::recvEat()
        {
            // est-ce que l'alien veut manger ?
            proxy->eatDecided.connect(this, &updateAlien::Operation::recvEatPost);
            proxy->queryEat(impl->turn);
        }

        void Operation::recvEatPost(operation::Status status,
                                    bool              eat)
        {
            proxy->eatDecided.disconnect(this, &updateAlien::Operation::recvEatPost);

            if (checkStatus(status))
            {
                if (eat)
                {
                    if (impl->gui->debug())
                    {
                        ostringstream oss;
                        oss << "Mange et gagne " << Simulation::EatingBonus <<
                            " énergie mais doit dormir";
                        impl->gui->appendDebug(oss.str());
                    }

                    impl->board->removeFood(nx, ny);
                    impl->board->moveAlien(alien, nx, ny);

                    // l'alien mange !
                    success(true);
                }
                else
                {
                    checkAlienAfterMove();
                }
            }
        }

        void Operation::checkAlienAfterMove()
        {
            oAlien = (*impl->board)(nx, ny);

            if (!oAlien.empty())
            {
                oProxy = impl->proxys[oAlien];
                if (oProxy.empty())
                {
                    throw SimulationException(
                              "Bug : un proxy sans alien, pas supposé de se produire...");
                    // on arrivera jamais la
                }

                // un autre alien !
                if (oAlien->realSpecies() == alien->realSpecies())
                {
                    // ... de la même espèce !
                    if (!alien->hasMated() &&
                        !oAlien->hasMated() &&
                        !oAlien->mating() &&
                        !oAlien->sleeping() &&
                        !oAlien->eating())
                    {
                        // partenaire trouve !
                        sendMate();
                    }
                    else
                    {
                        // impossible de se reproduire !
                        // on ne peut pas aller sur la case,
                        // le tour est perdu !
                        if (impl->gui->debug())
                        {
                            ostringstream oss;
                            oss << "Ne peut s'accoupler !" << endl;
                            impl->gui->appendDebug(oss.str());
                        }
                        dontMove();
                    }
                }
                else
                {
                    // espèce différente !!
                    if (oAlien->sleeping() ||
                        oAlien->mating() ||
                        oAlien->eating())
                    {
                        // si l'autre dort d'une quelconque manière, il perd instantannément
                        if (impl->gui->debug())
                        {
                            ostringstream oss;
                            oss << "Se bat contre l'alien d'id " << oAlien->id()
                                << " qui ne peut pas combattre et gagne automatiquement";
                            impl->gui->appendDebug(oss.str());
                        }
                        sendWin();
                    }
                    else
                    {
                        recvAttack();
                    }
                    // un des deux et enlevé du monde
                }
            }
            else
            {
                finallyMove();
            }
        }

        void Operation::sendMate()
        {
            // notifie les aliens qu'ils s'accouplent !
            proxy->mateDispatched.connect(this, &updateAlien::Operation::sendMateFirst);
            proxy->infoMate(impl->turn);
        }

        void Operation::sendMateFirst()
        {
            proxy->mateDispatched.disconnect(this,
                                             &updateAlien::Operation::sendMateFirst);

            oProxy->mateDispatched.connect(this, &updateAlien::Operation::sendMateSecond);
            oProxy->infoMate(impl->turn);
        }

        void Operation::sendMateSecond()
        {
            oProxy->mateDispatched.disconnect(this,
                                              &updateAlien::Operation::sendMateSecond);

            sendMatePost();
        }

        void Operation::sendMatePost()
        {
            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "S'accouple avec l'alien d'id " << oAlien->id();
                impl->gui->appendDebug(oss.str());
            }

            // choisit lequel des deux aliens produira les bébés !
            impl->matingAliens.insert(rand() % 2 == 0 ? alien : oAlien);

            // fini !
            success(true);
        }

        void Operation::sendWin()
        {
            proxy->winDispatched.connect(this, &updateAlien::Operation::sendWinPost);
            proxy->infoWin(impl->turn);
        }

        void Operation::sendWinPost()
        {
            proxy->winDispatched.disconnect(this, &updateAlien::Operation::sendWinPost);

            sendOtherLose();
        }

        void Operation::sendOtherLose()
        {
            oProxy->loseDispatched.connect(this,
                                           &updateAlien::Operation::sendOtherLosePost);
            oProxy->infoLose(impl->turn);
        }

        void Operation::sendOtherLosePost()
        {
            oProxy->loseDispatched.connect(this,
                                           &updateAlien::Operation::sendOtherLosePost);

            impl->board->removeAlien(oAlien);

            finallyMove();
        }

        void Operation::sendLose()
        {
            proxy->loseDispatched.connect(this, &updateAlien::Operation::sendLosePost);
            proxy->infoLose(impl->turn);
        }

        void Operation::sendLosePost()
        {
            proxy->loseDispatched.disconnect(this, &updateAlien::Operation::sendLosePost);

            sendOtherWin();
        }

        void Operation::sendOtherWin()
        {
            oProxy->winDispatched.connect(this,
                                          &updateAlien::Operation::sendOtherWinPost);
            oProxy->infoWin(impl->turn);
        }

        void Operation::sendOtherWinPost()
        {
            oProxy->winDispatched.disconnect(this,
                                             &updateAlien::Operation::sendOtherWinPost);

            // l'alien est mort, c'est fini :(
            impl->board->removeAlien(alien);

            success(true);
        }

        void Operation::recvAttack()
        {
            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Combat l'alien d'id " << oAlien->id();
                impl->gui->appendDebug(oss.str());
            }

            recvAttackFirst();
        }

        void Operation::recvAttackFirst()
        {
            proxy->attackDecided.connect(this,
                                         &updateAlien::Operation::recvAttackFirstPost);
            proxy->queryAttack(impl->turn,
                               oAlien->color(),
                               oAlien->species());
        }

        void Operation::recvAttackFirstPost(operation::Status status,
                                            Alien::Attack     attack)
        {
            proxy->attackDecided.disconnect(this,
                                            &updateAlien::Operation::recvAttackFirstPost);
            if (checkStatus(status))
            {
                attackAlien = attack;

                recvAttackSecond();
            }
        }

        void Operation::recvAttackSecond()
        {
            oProxy->attackDecided.connect(this,
                                          &updateAlien::Operation::recvAttackSecondPost);
            oProxy->queryAttack(impl->turn,
                                alien->color(),
                                alien->species());
        }

        void Operation::recvAttackSecondPost(operation::Status status,
                                             Alien::Attack     attack)
        {
            oProxy->attackDecided.disconnect(this,
                                             &updateAlien::Operation::recvAttackSecondPost);
            if (checkStatus(status))
            {
                attackOtherAlien = attack;

                checkFight();
            }
        }

        void Operation::checkFight()
        {
            // COMBAT !!
            bool win;
            if (attackAlien == attackOtherAlien) { win = rand() % 2; }
            else if (attackAlien == Alien::Forfeit)
            {
                win = false;
            }
            else if (attackOtherAlien == Alien::Forfeit)
            {
                win = true;
            }
            else
            {
                char table[] = " 011 001 ";
                win = static_cast<bool>(table[attackOtherAlien * 3 + attackAlien] - '0');
            }

            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Alien #1 choisit " << Alien::attackString(attackAlien)
                    << " et alien #2 choisit " << Alien::attackString(attackOtherAlien);
                impl->gui->appendDebug(oss.str());
            }

            if (win)
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "Gagne le combat !";
                    impl->gui->appendDebug(oss.str());
                }
                sendWin();
            }
            else
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "Perd le combat et meurt dans d'atroces souffrances !";
                    impl->gui->appendDebug(oss.str());
                }
                sendLose();
            }
        }

        void Operation::finallyMove()
        {
            // on se déplace gentiment
            impl->board->moveAlien(alien, nx, ny);
            if ((nx >= impl->board->width()) || (y >= impl->board->height()))
            {
                cerr << "BUGGG ! pas supposé de se produire !" << endl;
            }
            if (impl->gui->debug())
            {
                ostringstream oss;
                oss << "Perd " << Simulation::MovePenalty << " énergies";
                impl->gui->appendDebug(oss.str());
            }

            proxy->energyRemoved.connect(this, &updateAlien::Operation::finallyMovePost);
            proxy->removeEnergy(impl->turn, Simulation::MovePenalty);
        }

        void Operation::finallyMovePost()
        {
            proxy->energyRemoved.disconnect(this,
                                            &updateAlien::Operation::finallyMovePost);

            if (alien->energy() == 0)
            {
                if (impl->gui->debug())
                {
                    ostringstream oss;
                    oss << "N'a plus d'énergie et doit dormir";
                    impl->gui->appendDebug(oss.str());
                }
            }

            // fini !
            success(true);
        }
    }
}

void Simulation::Impl::update()
{
    if (started)
    {
        if (!sim->over() && !updating)
        {
            if (tick == -1)
            {
                beginUpdate();
            }
            else
            {
                doUpdate();
            }
        }
    }
    else
    {
        cerr <<
            "Simulation::update ne peut pas être appelé tant que la simulation n'a pas commencé !"
             << endl;
    }
}

namespace
{
    namespace beginUpdate
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation>(done),
            impl(impl)
        { }

        void Operation::run()
        {
            updateColorSpeciesLoopPre();
        }

        void Operation::updateColorSpeciesLoopPre()
        {
            index = 0;
            count = impl->currentAlienPerm.size();
            updateColorSpeciesLoop();
        }

        void Operation::updateColorSpeciesLoop()
        {
            if (index != count)
            {
                updateColorSpecies();
            }
            else
            {
                updateColorSpeciesLoopPost();
            }
        }

        void Operation::updateColorSpeciesLoopPost()
        {
            // Choisit un ordre d'update des aliens
            random_shuffle(impl->currentAlienPerm.begin(),
                           impl->currentAlienPerm.end());
            impl->tick++;
            success();
        }

        void Operation::updateColorSpecies()
        {
            bool       aware = false;
            ptr<Alien> alien = impl->currentAlienPerm[index];
            if (!alien.empty())
            {
                ptr<AlienProxy> proxy = impl->proxys[alien];
                if (proxy.empty())
                {
                    throw SimulationException(
                              "Bug : un proxy sans alien, pas supposé de se produire...");
                    // on arrivera jamais la
                }

                // trouve la position de l'alien !
                pair<int, int> pos = (*impl->board)[alien];
                int            x   = pos.first;
                int            y   = pos.second;

                if ((x >= 0) && (y >= 0))
                {
                    // pas de choix pour les aliens qui dorment !
                    if (!alien->sleeping() &&
                        !alien->eating() &&
                        !alien->mating())
                    {
                        aware = true;
                    }
                }
            }

            if (aware)
            {
                updateTurnPre();
            }
            else
            {
                updateColorSpeciesPost();
            }
        }

        void Operation::updateTurnPre()
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->turnDispatched.connect(this, &beginUpdate::Operation::updateTurn);
            proxy->infoTurn(impl->turn);
        }

        void Operation::updateTurn()
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->turnDispatched.disconnect(this, &beginUpdate::Operation::updateTurn);

            updateTurnPost();
        }

        void Operation::updateTurnPost()
        {
            updateColorPre();
        }

        void Operation::updateColorPre()
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->colorDecided.connect(this, &beginUpdate::Operation::updateColor);
            proxy->queryColor(impl->turn);
        }

        void Operation::updateColor(operation::Status status,
                                    Alien::Color)
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->colorDecided.disconnect(this, &beginUpdate::Operation::updateColor);

            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    updateDisqualify();
                    break;
                case operation::Success:
                    updateColorPost();
                    break;
            }
        }

        void Operation::updateDisqualify()
        {
            dis_op = make_ptr<disqualify::Operation>(impl);
            dis_op->done.connect(this, &beginUpdate::Operation::updateDisqualifyPost);
            dis_op->exec();
        }

        void Operation::updateDisqualifyPost(operation::Status status)
        {
            if (status != operation::Success)
            {
                fail();
            }
            else
            {
                dis_op->done.disconnect(this,
                                        &beginUpdate::Operation::updateDisqualifyPost);
                updateColorSpeciesPost();
            }
        }

        void Operation::updateColorPost()
        {
            updateSpeciesPre();
        }

        void Operation::updateSpeciesPre()
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->speciesDecided.connect(this, &beginUpdate::Operation::updateSpecies);
            proxy->querySpecies(impl->turn);
        }

        void Operation::updateSpecies(operation::Status status,
                                      Alien::Species)
        {
            ptr<Alien>      alien = impl->currentAlienPerm[index];
            ptr<AlienProxy> proxy = impl->proxys[alien];
            proxy->speciesDecided.disconnect(this,
                                             &beginUpdate::Operation::updateSpecies);

            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    updateDisqualify();
                    break;
                case operation::Success:
                    updateSpeciesPost();
                    break;
            }
        }

        void Operation::updateSpeciesPost()
        {
            updateAlienStatus();
        }

        void Operation::updateAlienStatus()
        {
            // juste pour updater couleur et espece !
            ptr<Alien> alien = impl->currentAlienPerm[index];
            // trouve la position de l'alien !
            pair<int, int> pos = (*impl->board)[alien];
            impl->gui->moveAlien(alien, pos.first, pos.second);

            updateColorSpeciesPost();
        }

        void Operation::updateColorSpeciesPost()
        {
            ++index;
            updateColorSpeciesLoop();
        }
    }
}

void Simulation::Impl::beginUpdate()
{
    updating = true;

    bool debug = gui->debug();
    // début de tour
    if (debug)
    {
        ostringstream oss;
        oss << "================" << endl;
        oss << "Tour " << turn << endl;
        gui->appendDebug(oss.str());
    }

    // Remet de la bouffe
    if (turn % Simulation::FoodRespawnTurns == 0)
    {
        if (board->full())
        {
            throw SimulationException("La carte est pleine!");
            // on arrivera jamais la..
        }
        pair<int, int> p = board->randomEmptySpot();
        board->addFood(p.first, p.second);
        if (debug)
        {
            ostringstream oss;
            oss << "Nouvelle nourriture a la case : (" << p.first << ","
                << p.second << ")";
            gui->appendDebug(oss.str());
        }
    }

    currentAlienPerm = board->aliens();

    beg_op = make_ptr<beginUpdate::Operation>(make_non_owning_ptr<Impl>(this));
    beg_op->done.connect(this, &Simulation::Impl::beginUpdatePost);
    beg_op->exec();
}

void Simulation::Impl::beginUpdatePost(operation::Status status)
{
    beg_op->done.disconnect(this, &Simulation::Impl::beginUpdatePost);

    // met à jour l'affichage
    gui->refresh();

    switch (status)
    {
        case operation::Failure:
            throw SimulationException(
                      "Une erreur s'est produite durant la mise à jour des aliens.");
            // on arrivera jamais la
            break;
        case operation::Timeout:
            // déjà traité !
            break;
        case operation::Success:
            doUpdate();
            break;
    }
}

void Simulation::Impl::doUpdate()
{
    updating = false;

    switch (gui->updateType())
    {
        case Gui::TICK:
            doTick();
            break;
        case Gui::TURN:
        case Gui::ALL:
            doTurn();
            break;
        case Gui::NONE:
            break;
    }
}

namespace
{
    namespace updateTurn
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation>(done),
            impl(impl)
        { }

        void Operation::run()
        {
            updateAlienLoopPre();
        }

        void Operation::updateAlienLoopPre()
        {
            count = impl->currentAlienPerm.size();
            updateAlienLoop();
        }

        void Operation::updateAlienLoop()
        {
            if (impl->tick != count)
            {
                updateAlien();
            }
            else
            {
                updateAlienLoopPost();
            }
        }

        void Operation::updateAlien()
        {
            upd_op = make_ptr<updateAlien::Operation>(impl);
            upd_op->done.connect(this, &updateTurn::Operation::updateAlienCheck);
            upd_op->exec();
        }

        void Operation::updateAlienCheck(operation::Status status,
                                         bool)
        {
            upd_op->done.disconnect(this, &updateTurn::Operation::updateAlienCheck);
            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    updateDisqualify();
                    break;
                case operation::Success:
                    updateAlienPost();
                    break;
            }
        }

        void Operation::updateDisqualify()
        {
            dis_op = make_ptr<disqualify::Operation>(impl);
            dis_op->done.connect(this, &updateTurn::Operation::updateDisqualifyPost);
            dis_op->exec();
        }

        void Operation::updateDisqualifyPost(operation::Status status)
        {
            if (status != operation::Success)
            {
                fail();
            }
            else
            {
                dis_op->done.disconnect(this,
                                        &updateTurn::Operation::updateDisqualifyPost);
                updateAlienPost();
            }
        }

        void Operation::updateAlienPost()
        {
            impl->tick++;
            updateAlienLoop();
        }

        void Operation::updateAlienLoopPost()
        {
            success();
        }
    }
}


void Simulation::Impl::doTurn()
{
    updating = true;

    upd_op = make_ptr<updateTurn::Operation>(make_non_owning_ptr<Impl>(this));
    upd_op->done.connect(this, &Simulation::Impl::doTurnPost);
    upd_op->exec();
}

void Simulation::Impl::doTurnPost(operation::Status status)
{
    upd_op->done.disconnect(this, &Simulation::Impl::doTurnPost);

    if (status != operation::Success)
    {
        throw SimulationException("Une erreur s'est produite durant la simulation!");
        // on arrivera jamais la
    }

    if (gui->debug())
    {
        ostringstream oss;
        oss << "Fin du tour " << turn;
        gui->appendDebug(oss.str());
    }

    turn++;
    tick = -1;

    updateStats();

    stat_op = make_ptr<updateSimStatus::Operation>(make_non_owning_ptr<Impl>(this));
    stat_op->done.connect(this, &Simulation::Impl::updateStatsPost);
    stat_op->exec();

}

void Simulation::Impl::updateStatsPost(operation::Status status)
{
    if (status != operation::Success)
    {
        throw SimulationException("Une erreur s'est produite durant la simulation!");
        // on arrivera jamais la
    }

    stat_op->done.disconnect(this, &Simulation::Impl::updateStatsPost);

    if (gui->updateType() == Gui::TURN)
    {
        // on a fait un tour !
        gui->setUpdateType(Gui::NONE);
    }

    endUpdate();
}

namespace
{
    namespace updateTick
    {
        Operation::Operation(ptr<Simulation::Impl> impl) :
            operation::Immediate<Operation>(done),
            impl(impl)
        { }

        void Operation::run()
        {
            updateAlienLoopPre();
        }

        void Operation::updateAlienLoopPre()
        {
            count = impl->currentAlienPerm.size();
            updateAlienLoop();
        }

        void Operation::updateAlienLoop()
        {
            if (impl->tick != count)
            {
                updateAlien();
            }
            else
            {
                success();
            }
        }

        void Operation::updateAlien()
        {
            upd_op = make_ptr<updateAlien::Operation>(impl);
            upd_op->done.connect(this, &updateTick::Operation::updateAlienCheck);
            upd_op->exec();
        }

        void Operation::updateAlienCheck(operation::Status status,
                                         bool              ret)
        {
            upd_op->done.disconnect(this, &updateTick::Operation::updateAlienCheck);
            switch (status)
            {
                case operation::Failure:
                    fail();
                    break;
                case operation::Timeout:
                    updateDisqualify();
                    break;
                case operation::Success:
                    updateAlienPost(ret);
                    break;
            }
        }

        void Operation::updateDisqualify()
        {
            dis_op = make_ptr<disqualify::Operation>(impl);
            dis_op->done.connect(this, &updateTick::Operation::updateDisqualifyPost);
            dis_op->exec();
        }

        void Operation::updateDisqualifyPost(operation::Status status)
        {
            if (status != operation::Success)
            {
                fail();
            }
            else
            {
                dis_op->done.disconnect(this,
                                        &updateTick::Operation::updateDisqualifyPost);
                updateAlienPost(true);
            }
        }

        void Operation::updateAlienPost(bool ret)
        {
            impl->tick++;
            if (ret) { success(); }
            else { updateAlienLoop(); }
        }
    }
}

void Simulation::Impl::doTick()
{
    updating = true;

    upt_op = make_ptr<updateTick::Operation>(make_non_owning_ptr<Impl>(this));
    upt_op->done.connect(this, &Simulation::Impl::doTickPost);
    upt_op->exec();
}

void Simulation::Impl::doTickPost(operation::Status status)
{
    upt_op->done.disconnect(this, &Simulation::Impl::doTickPost);

    if (status != operation::Success)
    {
        throw SimulationException("Une erreur s'est produite durant la simulation!");
        // on arrivera jamais la
    }

    // on a fait un tick !
    gui->setUpdateType(Gui::NONE);

    if (tick == currentAlienPerm.size())
    {
        if (gui->debug())
        {
            ostringstream oss;
            oss << "Fin du tour " << turn;
            gui->appendDebug(oss.str());
        }
        turn++;
        tick = -1;

        updateStats();

        stat_op = make_ptr<updateSimStatus::Operation>(make_non_owning_ptr<Impl>(this));
        stat_op->done.connect(this, &Simulation::Impl::updateStatsPost);
        stat_op->exec();
    }
    else
    {
        updating = false;
        // met à jour l'affichage
        gui->refresh();
    }
}

void Simulation::Impl::endUpdate()
{
    updating = false;

    // met à jour l'affichage
    gui->refresh();

    // schedule la prochaine update
    if (gui->updateType() == Gui::ALL)
    {
        int nextUpdateDelay = gui->updateTime();
        updater.scheduleNextUpdate(nextUpdateDelay);
    }
    // else : c'est le gui qui émettra les signaux pour appeler update
}

void Simulation::Impl::showMessage(const string &message)
{
#ifndef NACL
    gui->showMessage(message); // will block
    sim->msgAcked();
#endif
}

void Simulation::Impl::updateStats()
{
    vector<int> counts(Simulation::MaxNumSpecies);
    for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
    {
        Alien::Species sp = static_cast<Alien::Species>(i);
        counts[i] = board->countAliens(sp);
    }
    gui->setStats(counts, turn, board->countFood());
}

Simulation::Simulation(int width,
                       int height)
{
    m_impl = make_ptr<Impl>(make_non_owning_ptr<Simulation>(this), width, height);
}

int Simulation::width() const
{
    return priv(m_impl)->board->width();
}

int Simulation::height() const
{
    return priv(m_impl)->board->height();
}

const std::vector< ptr<Alien> > Simulation::aliens() const
{
    return priv(m_impl)->currentAlienPerm;
}

int Simulation::numPlayers() const
{
    return (priv(m_impl)->species.find(Alien::Grutub) != priv(m_impl)->species.end()) +
           (priv(m_impl)->species.find(Alien::Owa) != priv(m_impl)->species.end());
}

void Simulation::addLocalAlien(ptr<Alien> alien)
{
    // nombre d'espèces et du nombre d'aliens par espèces
    if (!priv(m_impl)->started)
    {
        ptr<LocalAlienProxy> proxy = make_ptr<LocalAlienProxy>(alien);
        priv(m_impl)->proxys[alien] = proxy;
        priv(m_impl)->currentAlienPerm.push_back(alien);
        priv(m_impl)->species.insert(alien->realSpecies());
    }
    else
    {
        cerr <<
            "Simulation::addLocalAlien ne peut pas être appelé après que la simulation ait commencé !"
             << endl;
    }
}

void Simulation::addRemoteAlien(ptr<Server> server,
                                int         playerId,
                                ptr<Alien>  alien)
{
    // nombre d'espèces et du nombre d'aliens par espèces
    if (!priv(m_impl)->started)
    {
        ptr<RemoteAlienProxy> proxy = make_ptr<RemoteAlienProxy>(server, playerId, alien);
        priv(m_impl)->proxys[alien] = proxy;
        priv(m_impl)->currentAlienPerm.push_back(alien);
        priv(m_impl)->species.insert(alien->realSpecies());
    }
    else
    {
        cerr <<
            "Simulation::addLocalAlien ne peut pas être appelé après que la simulation ait commencé !"
             << endl;
    }
}

void Simulation::start()
{
    if (!priv(m_impl)->started)
    {
        int width  = priv(m_impl)->board->width();
        int height = priv(m_impl)->board->height();

        size_t      numCells = width * height;
        vector<int> pos(numCells);
        for (size_t i = 0; i < numCells; ++i)
        {
            pos[i] = i;
        }
        random_shuffle(pos.begin(), pos.end());

        // Placement des aliens
        size_t i = 0;
        for (map< ptr<Alien>, ptr<AlienProxy> >::const_iterator it =
                 priv(m_impl)->proxys.begin();
             it != priv(m_impl)->proxys.end(); ++it, ++i)
        {
            priv(m_impl)->board->addAlien(it->first, pos[i] % width, pos[i] / width);
        }

        // Placement de la bouffe !
        size_t numOfFoods =
            static_cast<size_t>(Simulation::PercentageOfFood * numCells / 100. + 0.5);
        for (size_t j = 0; j < numOfFoods; ++j, ++i)
        {
            priv(m_impl)->board->addFood(pos[i] % width, pos[i] / width);
        }

        priv(m_impl)->updateStats();

        priv(m_impl)->gui->show();

        priv(m_impl)->started = true;

        priv(m_impl)->update();
    }
    else
    {
        cerr <<
            "Simulation::start ne peut pas être appelé après que la simulation ait commencé !"
             << endl;
    }
}

bool Simulation::started()
{
    return priv(m_impl)->started;
}

void Simulation::stop()
{
    priv(m_impl)->started = false;
    priv(m_impl)->over    = true;
}

bool Simulation::stopped()
{
    return !started();
}

bool Simulation::over()
{
    return !priv(m_impl)->gui->isVisible() || priv(m_impl)->over;
}

int Simulation::turn()
{
    return priv(m_impl)->turn;
}

void Simulation::update()
{
    return priv(m_impl)->update();
}

bool Simulation::updating() const
{
    return priv(m_impl)->updating;
}

void Simulation::showMessage(const string &msg)
{
    priv(m_impl)->showMessage(msg);
}
