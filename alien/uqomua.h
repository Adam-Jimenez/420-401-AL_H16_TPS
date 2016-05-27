#ifndef ALIEN_UQOMUA_H
#define ALIEN_UQOMUA_H

#include <iostream>
#include "alien/alien.h"

class AlienUqomua : public Alien
{
    public:
        AlienUqomua();
        ptr<Alien> clone() const { return make_ptr<AlienUqomua>(*this); }
        Attack queryAttack(Color   alienColor,
                           Species alienSpecies);
        Move queryMove();
        bool queryEat();
        Color queryColor();
        Species querySpecies();
        void infoStatus(int x, int y, int width, int height, int energy);
        void infoTurn(int turn);
    private:
        bool m_move;
};
#endif
