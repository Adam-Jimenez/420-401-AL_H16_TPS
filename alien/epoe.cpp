#include "alien/epoe.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienEpoe::AlienEpoe() : Alien(Epoe)
{ }

Alien::Attack AlienEpoe::queryAttack(Color   alienColor,
                                     Species alienSpecies)
{
    return Forfeit;
}

Alien::Move AlienEpoe::queryMove()
{
    return None;
}

bool AlienEpoe::queryEat()
{
    return false;
}

Alien::Color AlienEpoe::queryColor()
{
    return Green;
}

Alien::Species AlienEpoe::querySpecies()
{
    return Epoe;
}
