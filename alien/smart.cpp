#include "alien/smart.h"
#include <stdlib.h>
#include <iostream>

using namespace std;

SmartAlien::SmartAlien(Species species, int id) :
    Alien(species, id),
    m_direction(static_cast<Move>(rand()%4+1))
    { }

void SmartAlien::infoStatus(int x, int y, int width, int height, int energy){
    m_width = width;
    m_height = height;
    m_x = x;
    m_y = y;
    m_energy = energy;
}

void SmartAlien::infoTurn(int turn){
    m_turn = turn;
    m_randomKey = rand()%2;
    m_turnKey = m_turn%2;
    m_foodAround = false;
    m_enemyAround = false;
    m_allyAround = false;
    m_foodVector.clear();
    m_allyVector.clear();
    m_enemyVector.clear();
}

// a reorganiser
void SmartAlien::infoNeighboor(int x, int y, Color color, Species species, bool sleeping, bool mating, bool eating){
    if(color==Blue && species==realSpecies()){
        if(eating || sleeping || mating || m_turn == 0 ){
            m_allyAround = true;
            m_allyVector.push_back(pair<int, int>(x, y));
        } // else on sait pas, on l'ignore
    } else {
        if(!m_turnKey){
            if((color==Red && species==Og) || (color==Blue && species==Uqomua)){
                m_allyAround = true;
                m_allyVector.push_back(pair<int, int>(x, y));
            }else{
                m_enemyAround = true;
                m_enemyVector.push_back(pair<int, int>(x, y));
            }
        }else{
            if((color==Green && species==Yuhq) || (color==Yellow && species == Grutub)){
                m_allyAround = true;
                m_allyVector.push_back(pair<int, int>(x, y));
            }else{
                m_enemyAround = true;
                m_enemyVector.push_back(pair<int, int>(x, y));
            }
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
        if(m_turn<25){
            return Fungus;
        }
        return Plasma;
    }else if(alienColor==Gray && alienSpecies==Epoe){
        // ben la, si Epoe attaque cest une atk random... so bleh
        //if(mySpecies!=Epoe || myColor==Yellow || myColor==Blue)
    }
    return static_cast<Attack>(rand()%3+1);
}

Alien::Move SmartAlien::calculateDirectionTo(int x, int y){
    if(m_x == x && m_y == y) return None;
    // 49 - 1 = 48 | devrait etre -2
    // 3 - 1 = 2 | devrait etre 2
    int xDiff = x-m_x;
    int yDiff = y-m_y;
    if(xDiff > m_width/2) xDiff-=m_width;
    if(yDiff > m_width/2) yDiff-=m_width;

    if(abs(xDiff) > abs(yDiff)){ // bouger horizontalement
        if(xDiff>0)
            return Right;
        return Left;
    }else if(yDiff>0)
        return Up;
    return Down;
}

bool SmartAlien::findPairInAllies(pair<int, int> a_pair){
    size_t size = m_allyVector.size();
    // jai essaye avec un iterateur et find,
    // sa donne une grosse poutine d'erreures bizarre
    for(size_t i=0; i<size; i++){
        if(m_allyVector[i] == a_pair) return true;
    }
    return false;
}

bool SmartAlien::allyAt(Move move){
    int x = m_x;
    int y = m_y;
    if(move == Right) x++;
    else if(move == Left) x--;
    else if(move == Up) y++;
    else if(move == Down) y--;
    return findPairInAllies(pair<int, int>(x, y));
}

Alien::Move SmartAlien::queryMove()
{
    if(m_enemyAround){
        if(m_energy>10){ // bouge vers enemie
            int enemyIndex = rand()%m_enemyVector.size();
            pair<int, int> enemyPos = m_enemyVector[enemyIndex];
            m_direction = calculateDirectionTo(enemyPos.first, enemyPos.second);
            if(allyAt(m_direction)) return None;
        }else{ // conserve energie
            return None;
        }
    }else if(m_allyAround && m_energy > 10 && !hasMated()){
        int allyIndex = rand()%m_allyVector.size();
        pair<int, int> allyPos = m_allyVector[allyIndex];
        m_direction = calculateDirectionTo(allyPos.first, allyPos.second);
    }else if(m_foodAround && m_energy < 20){
        //bouge vers bouffe
        int foodIndex = rand()%m_foodVector.size();
        pair<int, int> foodPos = m_foodVector[foodIndex];
        m_direction = calculateDirectionTo(foodPos.first, foodPos.second);
    }else if(m_energy<20){
        return None;
    }

    return m_direction;
}

bool SmartAlien::queryEat()
{
    if(m_enemyAround){
        if(!m_allyAround || m_energy>20) return false;
        return true;
    }
    if(m_energy<20)
        return true;
    return false;
}

Alien::Color SmartAlien::queryColor()
{
    if((m_energy<15 && m_foodAround) || m_energy<5) return Purple;
    if(!m_turnKey) return m_randomKey? Red : Blue;
    return m_randomKey? Green : Yellow;
}

Alien::Species SmartAlien::querySpecies()
{
    if((m_energy<15 && m_foodAround) || m_energy<5) return Grutub;
    if(!m_turnKey) return m_randomKey? Og : Uqomua;
    return m_randomKey? Yuhq : Grutub;
}

