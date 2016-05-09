#include "alien/yuhq.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienYuhq::AlienYuhq() : Alien(Yuhq)
{ }

Alien::Attack AlienYuhq::queryAttack(Color   alienColor,
                                     Species alienSpecies)
{
    return Forfeit;
}

Alien::Move AlienYuhq::queryMove()
{
    return None;
}

bool AlienYuhq::queryEat()
{
    return false;
}

Alien::Color AlienYuhq::queryColor()
{
    return Gray;
}

Alien::Species AlienYuhq::querySpecies()
{
    return Yuhq;
}
