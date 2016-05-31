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

bool SmartAlien::isAlly(Color color, Species species, bool eating, bool sleeping, bool mating) const {
    if(color==Blue && species==realSpecies()){
        if(eating || sleeping || mating || m_turn == 0 ){
            return true;
        } else // else on sait pas, on l'ignore
            return false;
    } else {
        if(!m_turnKey){
            if((color==Red && species==Og) || (color==Blue && species==Uqomua)){
                return true;
            }else{
                return false;
            }
        }else{
            if((color==Green && species==Yuhq) || (color==Yellow && species == Grutub)){
                return true;
            }else{
                return false;
            }
        }
    }
}

void SmartAlien::infoNeighboor(int x, int y, Color color, Species species, bool sleeping, bool mating, bool eating){
    if(isAlly(color, species, sleeping, mating, eating)){
        m_allyVector.push_back(pair<int, int>(x, y));
        m_allyAround = true;
    } else {
        m_enemyVector.push_back(pair<int, int>(x, y));
        m_enemyAround = true;
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

Alien::Move SmartAlien::calculateDirectionTo(pair<int, int> p_pair) const{
    if(m_x == p_pair.first && m_y == p_pair.second) return None;
    pair<int, int> m_pair = getDistancePair(p_pair);

    if(abs(m_pair.first) > abs(m_pair.second)){ // bouger horizontalement
        if(m_pair.first>0)
            return Right;
        return Left;
    }else if(m_pair.second>0)
        return Up;
    return Down;
    /*
    int xDiff = x-m_x;
    int yDiff = y-m_y;
    if(xDiff > m_width/2) xDiff-=m_width;
    if(yDiff > m_height/2) yDiff-=m_width;
    */
}

bool SmartAlien::allyAt(Move move){
    int x = m_x;
    int y = m_y;

    if(move == Right) x++;
    else if(move == Left) x--;
    else if(move == Up) y++;
    else if(move == Down) y--;

    pair<int, int> a_pair(x,y);

    size_t size = m_allyVector.size();
    // jai essaye avec un iterateur et find,
    // sa donne une grosse poutine d'erreures bizarre
    for(size_t i=0; i<size; i++){
        if(m_allyVector[i] == a_pair) return true;
    }
    return false;
}

pair<int, int> SmartAlien::getDistancePair(pair<int, int> pPair) const{
    pair<int, int> mPair(pPair.first-m_x, pPair.second-m_y);
    if(mPair.first > m_width/2) mPair.first-=m_width;
    if(mPair.second > m_height/2) mPair.second-=m_height;
    return mPair;
}

pair<int, int> SmartAlien::getClosestObject(vector<pair<int, int> >& vec) {
    pair<int,int> rPair(0,0);
    if(vec.size()==0) return rPair;
    int tmp = 99999;
    int current;
    pair<int, int> tmp_pair;

    for(vector<pair<int, int> >::const_iterator it=vec.begin(); it!=vec.end(); it++){
        tmp_pair = getDistancePair(*it);
        current = abs(tmp_pair.first)+abs(tmp_pair.second);
        if(current < tmp){
            rPair = *it;
            tmp = current;
        }
    }

    return rPair;
}

Alien::Move SmartAlien::queryMove()
{
    if(m_enemyAround){
        if(m_energy>10){ // bouge vers enemie
            pair<int, int> enemyPos = getClosestObject(m_enemyVector);
            m_direction = calculateDirectionTo(enemyPos);
            if(allyAt(m_direction)) return None;
        }else{ // conserve energie
            return None;
        }
    }else if(m_allyAround && m_energy > 10 && !hasMated()){
        pair<int, int> allyPos = getClosestObject(m_allyVector);
        m_direction = calculateDirectionTo(allyPos);
    }else if(m_foodAround && m_energy < 20){
        //bouge vers bouffe
        pair<int, int> foodPos = getClosestObject(m_foodVector);
        m_direction = calculateDirectionTo(foodPos);
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

