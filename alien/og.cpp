#include "alien/og.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienOg::AlienOg() : Alien(Og)
{ }

Alien::Attack AlienOg::queryAttack(Color   alienColor,
                                   Species alienSpecies)
{

    return Forfeit;
}

Alien::Move AlienOg::queryMove()
{
    return None;
}

bool AlienOg::queryEat()
{
    return false;
}

Alien::Color AlienOg::queryColor()
{
    return Purple;
}

Alien::Species AlienOg::querySpecies()
{
    return Og;
}
