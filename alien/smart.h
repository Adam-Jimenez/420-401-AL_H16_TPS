#ifndef SMART_ALIEN_H
#define SMART_ALIEN_H

#include "alien/alien.h"
#include <vector>
#include <stdlib.h>

class SmartAlien : public Alien
{
    public:
        SmartAlien(Species species, int id=-1);
        ptr<Alien> clone() const { return make_ptr<SmartAlien>(*this); }
        Attack queryAttack(Color   alienColor,
                           Species alienSpecies);
        Move queryMove();
        bool queryEat();
        Color queryColor();
        Species querySpecies();

        void infoStatus(int x, int y, int width, int height, int energy);
        void infoTurn(int turn);
        void infoFood(int x, int y);
        void infoNeighboor(int x, int y, Color color, Species species, bool sleeping, bool mating, bool eating);
    private:
        int m_x;
        int m_y;
        int m_width;
        int m_height;
        int m_turn;
        int m_energy;
        bool m_randomKey;
        bool m_turnKey;

        bool m_foodAround;
        bool m_enemyAround;
        bool m_allyAround;

        Move m_direction;
        std::vector<std::pair<int, int> > m_foodVector;
        std::vector<std::pair<int, int> > m_allyVector;
        std::vector<std::pair<int, int> > m_enemyVector;

        bool isAlly(Color color, Species species, bool eating, bool sleeping, bool mating) const;
        Move calculateDirectionTo(const std::pair<int,int>& p_pair) const;
        bool allyAt(Move move) const;
        std::pair<int, int> getClosestObject(std::vector<std::pair<int, int> >& vec) const;
        std::pair<int, int> getDistancePair(const std::pair<int, int>& pPair) const;
};
#endif // SMART_ALIEN_H
