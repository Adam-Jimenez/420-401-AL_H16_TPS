#ifndef ALIEN_OG_H
#define ALIEN_OG_H

#include <iostream>
#include "alien/alien.h"

class AlienOg : public Alien
{
    public:
        AlienOg();
        ptr<Alien> clone() const { return make_ptr<AlienOg>(*this); }
        Attack queryAttack(Color   alienColor,
                           Species alienSpecies);
        Move queryMove();
        bool queryEat();
        Color queryColor();
        Species querySpecies();

    private:
};
#endif
