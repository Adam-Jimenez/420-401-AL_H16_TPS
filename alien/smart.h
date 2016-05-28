#ifndef SMART_ALIEN_H
#define SMART_ALIEN_H

#include "alien/alien.h"

class SmartAlien : public Alien
{
    public:
        SmartAlien(Species species,
                   int     id=-1);
        ptr<Alien> clone() const { return make_ptr<SmartAlien>(*this); }
        Attack queryAttack(Color   alienColor,
                           Species alienSpecies);
        Move queryMove();
        bool queryEat();
        Color queryColor();
        Species querySpecies();
        void infoTurn(int turn);
        void infoFood(int x, int y);
    private:
        int m_turn;
        bool m_foodAround;
        bool m_enemyAround;
        bool m_allyAround;
        Move m_direction;

};
#endif // SMART_ALIEN_H
