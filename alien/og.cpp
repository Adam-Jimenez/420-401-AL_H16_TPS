#include "alien/og.h"
#include <stdlib.h>

using namespace std;

AlienOg::AlienOg() : Alien(Og), m_step(0), m_count(0), m_direction(Up), m_xPivot(-1), m_yPivot(-1)
{ }

void AlienOg::infoTurn(int turn){
}

void AlienOg::infoStatus(int x,
                         int y,
                         int width,
                         int height,
                         int energy){
    if(m_xPivot==-1){
        m_xPivot = x;
        m_yPivot = y;
    }
    m_x = x;
    m_y = y;
}

Alien::Attack AlienOg::queryAttack(Color   alienColor,
                                   Species alienSpecies)
{
    if(alienSpecies==Uqomua)
        return Acid;
    else if(alienSpecies==Yuhq)
        return Plasma;
    else if(alienSpecies==Epoe)
        return Fungus;
    return Forfeit;
}

Alien::Move AlienOg::queryMove()
{
    // au debut, m_count=0, m_step=0 et m_direction est Up car Up-1 = Right (4)
    // (ou plutot None(0) mais on doit sauter par dessus lui)
    // donc au debut, la direction sera change a Right
    if(m_count == 0) 
        m_direction = (m_direction-5)%-4+4; 
    // m_step=0 au debut (et seulement au debut), donc init a 1 ici
    // et incremente chaque fois quil revient a son point d'origine
    // pour un maximum de 5
    if(m_x == m_xPivot && m_y == m_yPivot)
        m_step = m_step%5+1;
    // m_count compte le nombre de step effectue
    // premier tour, steps de 1, deuxieme, de deux, etc
    // quand il est a 0, changement de direction
    m_count = (m_count+1)%m_step;

    return static_cast<Move>(m_direction);
}

bool AlienOg::queryEat()
{
    if(m_step>=0 && m_step<=3)
        return true;
    return false;
}

Alien::Color AlienOg::queryColor()
{
    return Yellow;
}

Alien::Species AlienOg::querySpecies()
{
    return static_cast<Species>(rand()%6);
}
