#include "alien/uqomua.h"
#include <stdlib.h>

// TODO : Implémenter les comportements prédéfinis de ces aliens
// tel que décris dans l'énoncé.

// TODO : bug bizarre, desfois il ne change pas de couleur, surtout
// quand il lui manque de l'energie pour bouger

// TODO : bug GENERAL, desfois, le programme lance une exception
// du a des coordonnees inconsistentes/illogiques

using namespace std;

AlienUqomua::AlienUqomua() : Alien(Uqomua), m_move(rand()%2)
{ }

void AlienUqomua::infoStatus(int x, int y, int width, int height, int energy){
    cout << "infoStatus" << endl;
    m_move = !m_move;
}

void AlienUqomua::infoTurn(int turn){
    cout << "infoTurn" << endl;
}

Alien::Attack AlienUqomua::queryAttack(Color   alienColor,
                                       Species alienSpecies)
{
    cout << "query attack" << endl;
    if(alienColor==Red || alienColor==Green){
        if(alienSpecies!=Uqomua){
            return Acid;
        }
    } else if(alienColor==Yellow || alienColor==Gray){
        return Plasma;
    } else return Fungus;

    return Forfeit;
}

Alien::Move AlienUqomua::queryMove()
{
    cout << "query move" << endl;
    return m_move? Right : Down ;
}

bool AlienUqomua::queryEat()
{
    cout << "query eat" << endl;
    return true;
}

Alien::Color AlienUqomua::queryColor()
{
    cout << "query color" << endl;
    return m_move? Green : Red;
}

Alien::Species AlienUqomua::querySpecies()
{
    cout << "query species" << endl;
    return Uqomua;
}

