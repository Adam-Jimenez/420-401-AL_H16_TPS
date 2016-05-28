#include "alien/smart.h"
#include "stdlib.h"

using namespace std;

// TODO : Ajoutez ici le code de votre alien intelligent !
SmartAlien::SmartAlien(Species species, int id) :
    Alien(species, id),
    m_direction(static_cast<Move>(rand()%4+1)){ }

void SmartAlien::infoStatus(int x, int y, int width, int height, int energy){
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;
    m_energy = energy;
}

void SmartAlien::infoTurn(int turn){
    m_turn = turn;
    m_foodAround = false;
    m_enemyAround = false;
    m_foodVector.clear();
    m_allyVector.clear();
    m_enemyVector.clear();
}

// faut ajouter ds les vecteurs respectifs
void SmartAlien::infoNeighboor(int x, int y, Color color, Species species, bool sleeping, bool mating, bool eating){
    int key = m_turn%2;
    if(color==Purple && species==Owa){
        if(eating || sleeping || mating){
            m_allyAround = true;
        } // else on sait pas
    } else {
        if(key==0){
            if((color==Red && species==Og) || (color==Blue && species==Uqomua)){
                m_allyAround = true;
            }else m_enemyAround = true;
        }else{
            if((color==Green && species==Yuhq) || (color==Yellow && species == Grutub))
                m_allyAround = true;
            else m_enemyAround = true;
        }
    }
}


void SmartAlien::infoFood(int x, int y){
    m_foodVector.push_back(pair<int, int>(x, y));
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
        if(m_enemyAround && m_energy>15){
            //bouge dans direction opposee
        }
    } else {
        if(m_energy<15){
            //bouge vers bouffe
        }else if(m_enemyAround){
            //bouge vers enemy
        }
    }
    return m_direction;
}

bool SmartAlien::queryEat()
{
    if(m_enemyAround && !m_allyAround)
        return false;
    if(m_energy<10)
        return true;
    return false;
}

Alien::Color SmartAlien::queryColor()
{
    if(m_energy<10) return Purple;
    bool randomKey = rand()%2;
    int turnKey = m_turn%2;
    if(turnKey==0) return randomKey? Red : Blue;
    return randomKey? Green : Yellow;
}

Alien::Species SmartAlien::querySpecies()
{
    if(m_energy<10) return Owa;
    bool randomKey = rand()%2;
    int turnKey = m_turn%2;
    if(turnKey==0) return randomKey? Og : Uqomua;
    return randomKey? Yuhq : Grutub;
}

