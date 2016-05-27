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

        void infoTurn(int turn);
        void infoStatus(int x, int y, int width, int height, int energy);

    private:
        int m_step;
        int m_count;
        int m_direction;
        int m_xPivot;
        int m_yPivot;
        int m_x;
        int m_y;

};
#endif
