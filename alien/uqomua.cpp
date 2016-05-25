#include "alien/uqomua.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienUqomua::AlienUqomua() : Alien(Uqomua)
{ }

Alien::Attack AlienUqomua::queryAttack(Color   alienColor,
                                       Species alienSpecies)
{
    if(alienColor==Red || alienColor==Green){
        if(alienSpecies!=Uqomua){
            return Acid;
        }
    } else if(alienColor==Yellow || alienColor==Gray){
        return Plasma;
    } else return Fungus;

    return Forfeit;
}

// faut faire verifs? (si mange, dort, etc)
Alien::Move AlienUqomua::queryMove()
{
    return (m_move=!m_move)? Right : Down ;
}

bool AlienUqomua::queryEat()
{
    return true;
}

Alien::Color AlienUqomua::queryColor()
{
    return Color() == Red? Green : Red;
}

Alien::Species AlienUqomua::querySpecies()
{
    return Uqomua;
}
