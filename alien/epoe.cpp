#include "alien/epoe.h"

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

using namespace std;

AlienEpoe::AlienEpoe() : Alien(Epoe)
{
    m_move_counter=0;
}

Alien::Attack AlienEpoe::queryAttack(Color   alienColor,
                                     Species alienSpecies)
{
    Attack attack;
    if(alienColor == Yellow || alienColor == Blue || AlienSpecies != Epoe){
    attack = static_cast<Attack>(rand() % 3);
    }else{
        attack = Forfeit;
    }
    return attack;
}

Alien::Move AlienEpoe::queryMove()
{
    Move move;
    if(m_move_counter>=5){
        random_move=!random_move;
        m_move_counter=0;
        if(!random_move){
            direction = static_cast<Move>(rand()%4+1);
        }
    }

    if(random_move){
            move = static_cast<Move>(rand()%4+1);
    }else{
       move = direction; 
    }


    m_move_counter++;
    return move;
}

bool AlienEpoe::queryEat()
{
    return false;
}

Alien::Color AlienEpoe::queryColor()
{
    return Gray;
}

Alien::Species AlienEpoe::querySpecies()
{
    return Epoe;
}
