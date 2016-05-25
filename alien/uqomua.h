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

    private:
        bool m_move;
};
#endif
