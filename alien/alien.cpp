#include <sstream>
#include <iostream>
#include "alien/alien.h"
#include "src/simulation.h"
#include "server/server.h"

using namespace std;

void Alien::infoTurn(int turn){}

void Alien::infoStatus(int x,
                       int y,
                       int width,
                       int height,
                       int energy){
};


const string & Alien::moveString(Alien::Move move)
{
    static string moveStr[] =
    {
        "aucun", "haut", "gauche", "bas", "droit"
    };
    static string empty = "";

    if ((move >= None) && (move <= Right))
    {
        return moveStr[move];
    }

    return empty;
}

const string & Alien::colorString(Alien::Color col)
{
    static string colorStr[] =
    {
        "bleu", "violet", "gris", "jaune", "rouge", "vert", "orange"
    };
    static string empty = "";

    if ((col >= Blue) && (col <= Orange))
    {
        return colorStr[col];
    }

    return empty;
}

const string & Alien::speciesString(Alien::Species sp)
{
    static string speciesStr[] =
    {
        "Uquoma", "Og", "Yuhq", "Epoe", "Grutub", "Owa"
    };
    static string empty = "";

    if ((sp >= Uqomua) && (sp <= Owa))
    {
        return speciesStr[sp];
    }

    return empty;
}

const string & Alien::attackString(Alien::Attack attack)
{
    static string attackStr[] =
    {
        "Plasma", "Acide", "Spore"
    };
    static string forfeit = "Forfait";

    if ((attack >= Plasma) && (attack <= Fungus))
    {
        return attackStr[attack];
    }

    return forfeit;
}

set<int> Alien::m_ids;

Alien::Alien(Species species,
             int     id) :
    m_id(id),
    m_realSpecies(species),
    m_energy(Simulation::MaxEnergy),
    m_hasMated(false),
    m_sleepingTurn(-1),
    m_matingTurn(-1),
    m_eatingTurn(-1),
    m_species(m_realSpecies), // va changer éventuellement !
    m_color(Alien::Blue) // va changer éventuellement !
{
    if (m_id < 0)
    {
        if (m_ids.empty()) { m_id = 0; }
        else { m_id = *(--m_ids.end()) + 1; }
        m_ids.insert(m_id);
    }
    else
    {
        // vérifier que l'id n'existe pas déjà !
        if (m_ids.find(m_id) != m_ids.end())
        {
            stringstream ss;
            ss << "L'alien d'id " << m_id << " existe déjà!";
            throw InvalidAlienIdException(ss.str());
        }
    }
}

Alien::Alien(const Alien &alien) :
    m_realSpecies(alien.m_realSpecies),
    m_energy(alien.m_energy),
    m_hasMated(alien.m_hasMated),
    m_sleepingTurn(alien.m_sleepingTurn),
    m_matingTurn(alien.m_matingTurn),
    m_eatingTurn(alien.m_eatingTurn),
    m_species(alien.m_species), // va changer éventuellement !
    m_color(alien.m_color) // va changer éventuellement !
{
    m_id = *(--m_ids.end()) + 1;
    m_ids.insert(m_id);
}

Alien & Alien::operator=(const Alien &alien)
{
    if (this != &alien)
    {
        m_realSpecies  = alien.m_realSpecies;
        m_energy       = alien.m_energy;
        m_hasMated     = alien.m_hasMated;
        m_sleepingTurn = alien.m_sleepingTurn;
        m_matingTurn   = alien.m_matingTurn;
        m_eatingTurn   = alien.m_eatingTurn;
        m_species      = alien.m_species; // va changer éventuellement !
        m_color        = alien.m_color; // va changer éventuellement !

        m_id = *(--m_ids.end()) + 1;
        m_ids.insert(m_id);
    }

    return *this;
}

Alien::~Alien()
{
    m_ids.erase(m_id);
}

bool Alien::eating() const
{
    return m_eatingTurn >= 0;
}

int Alien::eatingTurn() const
{
    return m_eatingTurn;
}

void Alien::setEatingTurn(int turn)
{
    m_eatingTurn = turn;
}

int Alien::energy() const
{
    return m_energy;
}

void Alien::setEnergy(int energy)
{
    m_energy = energy;
}

bool Alien::sleeping() const
{
    return m_sleepingTurn >= 0;
}

int Alien::sleepingTurn() const
{
    return m_sleepingTurn;
}

void Alien::setSleepingTurn(int turn)
{
    m_sleepingTurn = turn;
}

bool Alien::hasMated() const
{
    return m_hasMated;
}

bool Alien::mating() const
{
    return m_matingTurn >= 0;
}

int Alien::matingTurn() const
{
    return m_matingTurn;
}

void Alien::setMatingTurn(int turn)
{
    m_matingTurn = turn;
    if (turn >= 0)
    {
        m_hasMated   = true;
    }
}

int Alien::id() const
{
    return m_id;
}

Alien::Species Alien::realSpecies() const
{
    return m_realSpecies;
}

Alien::Color Alien::color() const
{
    return m_color;
}

void Alien::setColor(Alien::Color col)
{
    m_color = col;
}

Alien::Species Alien::species() const
{
    return m_species;
}

bool Alien::smart() const
{
    return m_realSpecies == Grutub || m_realSpecies == Owa;
}

void Alien::setSpecies(Alien::Species species)
{
    m_species = species;
}
