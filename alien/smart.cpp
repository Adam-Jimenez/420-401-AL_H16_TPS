#include "alien/smart.h"
#include "stdlib.h"

using namespace std;

// TODO : Ajoutez ici le code de votre alien intelligent !
SmartAlien::SmartAlien(Species species, int id) :
    Alien(species, id),
    m_direction(static_cast<Move>(rand()%4+1)){ }

void SmartAlien::infoTurn(int turn){
    m_turn = turn;
}

void SmartAlien::infoFood(int x, int y){
    m_foodAround = true;
}

Alien::Attack SmartAlien::queryAttack(Color   alienColor,
                                      Species alienSpecies)
{
    // peut etre quon va mettre une var membre pour myColor/mySpecies
    // si elle se voit necessaire a d'autres fonctions
    Color myColor = color();
    Species mySpecies = species();
    if((alienColor==Red || alienColor==Green) && alienSpecies == Uqomua){
        if(myColor==Red || myColor==Green) return Plasma;
        else if (myColor==Yellow || myColor==Gray) return Fungus;
        else return Acid;
    }else if(alienColor==Yellow){ 
        // forte chance que ce soit Og
        // faut s'arranger pour faire en sorte que si on
        // l'attaque, que notre species soit Og, Grutub ou Owa
        // car il forfeit si c le cas
        if(mySpecies == Uqomua){
            return Plasma;
        }else if(mySpecies == Yuhq){
            return Fungus;
        }else if(mySpecies == Epoe){
            return Acid;
        }
    }else if(alienSpecies==Yuhq || alienSpecies==Og){
        // verifier si il a bouger recemment?
        return Plasma;
    }else if(alienColor==Gray && alienSpecies==Epoe){
        // ben la, si Epoe attaque cest une atk random... so bleh
        //if(mySpecies!=Epoe || myColor==Yellow || myColor==Blue)
    }
    return static_cast<Attack>(rand()%3+1);
}

Alien::Move SmartAlien::queryMove()
{
    if(!m_allyAround){
        if(m_enemyAround && energy()>20){
            //bouge dans direction opposee
        }
    } else {
        if(energy()<20){
            //bouge vers bouffe
        }else if(m_enemyAround){
            //bouge vers enemy
        }
    }
    return m_direction;
}

bool SmartAlien::queryEat()
{
    if(m_enemyAround){
        if(!m_allyAround || energy()>20) return false;
    }
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

