#include "alien/smart.h"

using namespace std;

// TODO : Ajoutez ici le code de votre alien intelligent !
SmartAlien::SmartAlien(Species species,
                       int     id) : Alien(species, id) { }


Alien::Attack SmartAlien::queryAttack(Color   alienColor,
                                      Species alienSpecies)
{
    return Forfeit;
}

Alien::Move SmartAlien::queryMove()
{
    return None;
}

bool SmartAlien::queryEat()
{
    return true;
}

Alien::Color SmartAlien::queryColor()
{
    return Red;
}

Alien::Species SmartAlien::querySpecies()
{
    return Grutub;
}

