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
};
#endif // SMART_ALIEN_H
