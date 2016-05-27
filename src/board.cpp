#include <vector>
#include <cstdlib>
#include <iostream>
#include "src/gui.h"
#include "src/board.h"
#include "alien/alien.h"
#include <time.h>

using namespace std;

// TODO faire les verification de pointers (? ==0)

Board::Board(int      width,
             int      height,
             ptr<Gui> gui) :
    m_width(width), m_height(height),
    m_gui(gui)
{
    // TODO! Complétez-moi
    // Allouer la grille de nourriture !
    m_food.resize(m_width*m_height);
}

void Board::setWidth(int width)
{
    // TODO! Complétez-moi
    // Assurez-vous que la largeur fasse du sens
    // Ré-allouer la grille de nourriture si nécessaire !
    if(width>=0) m_width=width;
    //realloue m_food
    m_food.clear();
    m_food.resize(m_width*m_height);
}

int Board::width() const
{
    // TODO! Complétez-moi
    return m_width;
}

void Board::setHeight(int height)
{
    // TODO! Complétez-moi
    // Assurez-vous que la hauteur fasse du sens
    // Ré-allouer la grille de nourriture si nécessaire !
    if(height>=0) m_height=height;
    //realloue m_food
    m_food.clear();
    m_food.resize(m_width*m_height);
}

int Board::height() const
{
    // TODO! Complétez-moi
    return m_height;
}

void Board::resize(int width,
                   int height)
{
    // TODO! Complétez-moi
    if(width>=0) m_width=width;
    if(height>=0) m_height=height;
    //realloue m_food
    m_food.clear();
    m_food.resize(m_width*m_height);
}

bool Board::full() const
{
    // TODO! Complétez-moi

    // Regarde si le nombre d'aliens + nombre de nourriture = taille de la carte
    return (m_food.size()*m_aliensToPos.size())==(m_width*m_height);
}

ptr<Alien> Board::operator()(int x,
                             int y) const
{
    // TODO! Complétez-moi
    // Retourne l'alien à la case (x,y) ou le pointeur nul le cas échéant
    // Servez-vous de pairToPos !
    map<int, ptr<Alien> >::const_iterator alienIt = m_posToAliens.find(pairToPos(x,y));
    return alienIt == m_posToAliens.end()? ptr<Alien>() : alienIt->second;
}

pair<int, int> Board::operator[](ptr<Alien> alien) const
{
    // TODO! Complétez-moi
    // Retourne une paire (x,y) représentant la case où l'alien est stocké
    // dans la carte ou la paire (-1, -1) le cas échéant
    // Servez-vous de posToPair !
    int x=-1, y=-1, pos;
    std::map< ptr<Alien>, int>::const_iterator it = m_aliensToPos.find(alien);
    if(it == m_aliensToPos.end())
        return make_pair(x, y);
    pos = it->second;
    posToPair(pos, x, y); 
    //cout << "operator x: " << x << "; y: " << y << endl;
    return make_pair(x, y);
}

vector< ptr<Alien> > Board::neighboors(ptr<Alien> alien) const
{
    // TODO! Complétez-moi
    // Retourne tous les aliens autour de l'alien passé en paramètre
    // dans un rayon de 3 cases.
    // Utilisez la formule sqrt((x1-x2)^2 + (y1-y2)^2) < 3
    // où l'alien passé en paramètre est à la case (x1,y1) et l'autre alien
    // à la case (x2,y2)

    vector< ptr<Alien> > aliens;
    pair<int, int> alienPair = (*this)[alien];
    int x1=alienPair.first, y1=alienPair.second; 
    int x2, y2; 
    
    for(std::map<int, ptr<Alien> >::const_iterator i=m_posToAliens.begin(); i!=m_posToAliens.end(); i++){
        if(i->second==alien) continue;
        //int x2, y2; ?
        alienPair = (*this)[i->second];
        x2=alienPair.first;
        y2=alienPair.second;
        
        if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 3){ 
            aliens.push_back(i->second);
        }
    }   
    
    return aliens;
}

vector< pair<int, int> > Board::foods(ptr<Alien> alien) const
{
    // TODO! Complétez-moi
    // Retourne toute la nourriture autour de l'alien passé en paramètre
    // dans un rayon de 3 cases.
    // Utilisez la formule sqrt((x1-x2)^2 + (y1-y2)^2) < 3
    // où l'alien passé en paramètre est à la case (x1,y1) et la nourriture
    // à la case (x2,y2)
    vector< pair<int, int> > foodVector;
    pair<int, int> alienPair = (*this)[alien];
    int x1 = alienPair.first, y1=alienPair.second;
    int x2, y2;

    for(int i = 0; i<m_food.size(); i++){
        if(m_food[i]){
            posToPair(i, x2, y2);

            if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 3){ 
                foodVector.push_back(make_pair(x2,y2));
            }
        }
    }


    return foodVector;
}

vector< ptr<Alien> > Board::aliens() const
{
    // TODO! Complétez-moi
    // Retourne tous les aliens sur la carte (peu importe l'ordre)
    vector< ptr<Alien> > aliens;
    for(map<int, ptr<Alien> >::const_iterator i=m_posToAliens.begin(); i!=m_posToAliens.end(); i++)
        aliens.push_back(i->second);
    return aliens;
}

bool Board::foodAt(int x,
                   int y) const
{
    // TODO! Complétez-moi
    // Retourne vraie si il y a de la nourriture à la case (x,y)
    // faux sinon
    int pos = pairToPos(x, y);

    return m_food[pos];
}

void Board::addFood(int x,
                    int y)
{
    // TODO! Complétez-moi
    // Ajoute de la nourriture dans la carte à la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->addFood(x, y); }
    int pos = pairToPos(x, y);
    m_food[pos] = true;
}

void Board::addAlien(ptr<Alien> alien,
                     int        x,
                     int        y)
{
    // TODO! Complétez-moi
    // Ajoute un alien dans la carte à la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->addAlien(alien, x, y); }
    int pos = pairToPos(x, y);
    // TODO
    // est-ce quil faut faire une copie de pointer ici??
    // et dans les autres fonctions??
    // ou bien cest loperateur = qui soccupe de sa?
    m_aliensToPos[alien] = pos;
    m_posToAliens[pos] = alien;
}

void Board::removeFood(int x,
                       int y)
{
    // TODO! Complétez-moi
    // Enlève de la nourriture de la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->removeFood(x, y); }
    int pos = pairToPos(x, y);
    m_food[pos] = false;
}

void Board::removeAlien(ptr<Alien> alien)
{
    // TODO! Complétez-moi
    // Enlève un alien de la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->removeAlien(alien); }
    int pos = m_aliensToPos[alien];
    m_aliensToPos.erase(alien);
    m_posToAliens.erase(pos);
}

void Board::moveAlien(ptr<Alien> alien,
                      int        x,
                      int        y)
{
    // TODO! Complétez-moi
    // Déplace un alien depuis sa case actuelle vers la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->moveAlien(alien, x, y); }
    int oldPos = m_aliensToPos[alien];
    int newPos = pairToPos(x, y);
    // TODO faut verifier le comportement de la copie de alien
    m_aliensToPos[alien] = newPos;
    m_posToAliens[newPos] = alien;
    m_posToAliens.erase(oldPos);
}

class isSameSpecies{
    private:
        Alien::Species m_species;

    public:
        isSameSpecies(Alien::Species species) : m_species(species) {};

        bool operator()(const ptr<Alien> alien) const{
            return m_species==alien->realSpecies();
        }
};

int Board::countAliens(Alien::Species species) const
{
    // TODO! Complétez-moi
    // Compte et retourne le nombre d'aliens d'une certaine espèce
    // Vous devez utiliser un algo de la STL et un foncteur qui
    // prend en paramètre l'espèce voulue pour écrire cette fonction
    vector< ptr<Alien> > aliens = this->aliens();
    int nb = count_if(aliens.begin(), aliens.end(), isSameSpecies(species));
    return nb;
}

int Board::countFood() const
{
    // TODO! Complétez-moi
    // Compte et retourne le nombre de bouffe
    // Vous devez utiliser un algo de la STL pour écrire cette fonction
    int nb = count(m_food.begin(), m_food.end(), 1);
    return nb;
}

class randomPair{
    private:
        int m_xmax, m_ymax;
    public:
        randomPair(int xmax, int ymax) : m_xmax(xmax), m_ymax(ymax) {
            srand(time(NULL));
        }

        pair<int, int> operator()(){
            return pair<int, int>(rand()%m_xmax, rand()%m_ymax);
        }
};

pair<int, int> Board::randomEmptySpot() const
{
    // TODO! Complétez-moi
    // Retourne une case au hasard qui ne contient ni alien
    // ni nourriture.
    randomPair rPair(m_width, m_height);
    pair<int, int> randomSpot = rPair();
    int pos = pairToPos(randomSpot.first, randomSpot.second);
    map<int, ptr<Alien> >::const_iterator findIt = m_posToAliens.find(pos);
    while(findIt != m_posToAliens.end() || m_food[pos]){
        randomSpot = rPair();
        pos = pairToPos(randomSpot.first, randomSpot.second);
        findIt = m_posToAliens.find(pos);
    }
    return randomSpot;
}

// Fonction utilitaire pour vous aider !
int Board::pairToPos(int x,
                     int y) const
{
    return y * m_width + x;
}

// Fonction utilitaire pour vous aider !
void Board::posToPair(int  pos,
                      int &x,
                      int &y) const
{
    y = pos / m_width;
    x = pos % m_width;
}
