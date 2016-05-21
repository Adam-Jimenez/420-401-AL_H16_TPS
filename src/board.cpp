#include <vector>
#include <cstdlib>
#include <iostream>
#include "src/gui.h"
#include "src/board.h"
#include "alien/alien.h"

using namespace std;

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
    return m_posToAliens.at(pairToPos(x,y));
    //return ptr<Alien>();
}

pair<int, int> Board::operator[](ptr<Alien> alien) const
{
    // TODO! Complétez-moi
    // Retourne une paire (x,y) représentant la case où l'alien est stocké
    // dans la carte ou la paire (-1, -1) le cas échéant
    // Servez-vous de posToPair !
    int x;
    int y;
    posToPair(m_aliensToPos(alien), x, y);
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
    vector< ptr<Alien> > aliens();
    int x1, y1;
    posToPair(m_aliensToPos(alien), x1, y1);
    for(std::map<int, ptr<Alien>>::const_iterator i=m_posToAliens.begin(); i!=m_posToAliens.end(); i++){
        if((*i)==alien) continue;
        int x2, y2;
        posToPair(m_aliensToPos(*it), x2, y2);
        if(sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)) < 3){
            aliens.push_back(*i);
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

    return vector< pair<int, int> >();
}

vector< ptr<Alien> > Board::aliens() const
{
    // TODO! Complétez-moi
    // Retourne tous les aliens sur la carte (peu importe l'ordre)

    return vector< ptr<Alien> >();
}

bool Board::foodAt(int x,
                   int y) const
{
    // TODO! Complétez-moi
    // Retourne vraie si il y a de la nourriture à la case (x,y)
    // faux sinon

    return false;
}

void Board::addFood(int x,
                    int y)
{
    // TODO! Complétez-moi
    // Ajoute de la nourriture dans la carte à la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->addFood(x, y); }
}

void Board::addAlien(ptr<Alien> alien,
                     int        x,
                     int        y)
{
    // TODO! Complétez-moi
    // Ajoute un alien dans la carte à la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->addAlien(alien, x, y); }
}

void Board::removeFood(int x,
                       int y)
{
    // TODO! Complétez-moi
    // Enlève de la nourriture de la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->removeFood(x, y); }
}

void Board::removeAlien(ptr<Alien> alien)
{
    // TODO! Complétez-moi
    // Enlève un alien de la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->removeAlien(alien); }
}

void Board::moveAlien(ptr<Alien> alien,
                      int        x,
                      int        y)
{
    // TODO! Complétez-moi
    // Déplace un alien depuis sa case actuelle vers la case (x,y)
    // Laissez ce code
    if (!m_gui.empty()) { m_gui->moveAlien(alien, x, y); }
}

int Board::countAliens(Alien::Species species) const
{
    // TODO! Complétez-moi
    // Compte et retourne le nombre d'aliens d'une certaine espèce
    // Vous devez utiliser un algo de la STL et un foncteur qui
    // prend en paramètre l'espèce voulue pour écrire cette fonction

    return 0;
}

int Board::countFood() const
{
    // TODO! Complétez-moi
    // Compte et retourne le nombre d'aliens d'une certaine espèce
    // Vous devez utiliser un algo de la STL pour écrire cette fonction

    return 0;
}

pair<int, int> Board::randomEmptySpot() const
{
    // TODO! Complétez-moi
    // Retourne une case au hasard qui ne contient ni alien
    // ni nourriture.

    return make_pair(0, 0);
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
