#include "alien/yuhq.h"
#include <stdlib.h>

using namespace std;

AlienYuhq::AlienYuhq() : Alien(Yuhq), m_moved(false), m_eatCount(-1)
{ }

void AlienYuhq::infoTurn(int turn){
    m_eatCount++;
}

Alien::Attack AlienYuhq::queryAttack(Color   alienColor,
                                     Species alienSpecies)
{
    if(m_moved) return Plasma;
    return Acid;
}

Alien::Move AlienYuhq::queryMove()
{
    if(m_eatCount<3) {
        m_moved = true;
        return static_cast<Move>(rand()%4+1);
    }
    m_moved = false;
    return None;
}

bool AlienYuhq::queryEat()
{
    if(m_eatCount>=3){
        m_moved = false;
        m_eatCount = 0;
        return true;
    }
    return false;
}

Alien::Color AlienYuhq::queryColor()
{
    return static_cast<Color>(rand()%7);
}

Alien::Species AlienYuhq::querySpecies()
{
    return (species() == Yuhq ? Og : Yuhq);
}
