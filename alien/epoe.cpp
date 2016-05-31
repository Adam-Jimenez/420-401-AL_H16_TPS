#include "alien/epoe.h"
#include <stdlib.h>

using namespace std;

AlienEpoe::AlienEpoe() : Alien(Epoe), random_move(true), m_fight_counter(4), m_move_counter(0)
{
}

void AlienEpoe::infoTurn(int turn){
    m_fight_counter++;
}

Alien::Attack AlienEpoe::queryAttack(Color   alienColor,
                                     Species alienSpecies)
{
    Attack attack;
    if(alienColor == Yellow || alienColor == Blue || alienSpecies != Epoe){
        attack = static_cast<Attack>(rand() % 3);
    }else{
        attack = Forfeit;
    }
    m_fight_counter = 0;
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
    if(energy()<25 || m_fight_counter<=3)
        return true;
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
