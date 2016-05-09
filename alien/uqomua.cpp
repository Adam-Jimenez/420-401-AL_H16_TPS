#include "alien/uqomua.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienUqomua::AlienUqomua() : Alien(Uqomua)
{ }

Alien::Attack AlienUqomua::queryAttack(Color   alienColor,
                                       Species alienSpecies)
{
    return Forfeit;
}

Alien::Move AlienUqomua::queryMove()
{
    return None;
}

bool AlienUqomua::queryEat()
{
    return false;
}

Alien::Color AlienUqomua::queryColor()
{
    return Red;
}

Alien::Species AlienUqomua::querySpecies()
{
    return Uqomua;
}
