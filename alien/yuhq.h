#ifndef ALIEN_YUHQ_H
#define ALIEN_YUHQ_H

#include <iostream>
#include "alien/alien.h"

class AlienYuhq : public Alien
{
    public:
        AlienYuhq();
        ptr<Alien> clone() const { return make_ptr<AlienYuhq>(*this); }
        Attack queryAttack(Color   alienColor,
                           Species alienSpecies);
        Move queryMove();
        bool queryEat();
        Color queryColor();
        Species querySpecies();
        void infoTurn(int turn);
        //void infoStatus(int x, int y, int width, int height, int energy);


    private:
        bool m_moved;
        int m_eatCount;
};
#endif
