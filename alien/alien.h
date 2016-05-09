#ifndef ALIEN_H
#define ALIEN_H

#include <string>
#include <set>
#include <stdexcept>
#include "src/ptr.h"

struct InvalidAlienIdException : public std::runtime_error
{
    InvalidAlienIdException(const std::string &what) :
        std::runtime_error(what) { }
};

class Alien
{
    public:
        // Les mouvements qu'un alien peut retourner dans la méthode move
        enum Move { None, Up, Left, Down, Right };
        // Convertit un mouvement en chaine pour l'affichage
        static const std::string & moveString(Move move);

        // Les couleurs qu'un alien peut retourner dans la méthode color
        enum Color { Blue, Purple, Gray, Yellow, Red, Green, Orange };
        // Convertit une couleur en chaine pour l'affichage
        static const std::string & colorString(Color col);

        // Les espèces qu'un alien peut retourner dans la méthode species
        enum Species { Uqomua, Og, Yuhq, Epoe, Grutub, Owa };
        // Convertit une espèce en chaine pour l'affichage
        static const std::string & speciesString(Species sp);

        // Les attaques qu'un alien peut retourner dans la méthode fight
        enum Attack { Plasma, Acid, Fungus, Forfeit };
        // Convertit une attaque en chaine pour l'affichage
        static const std::string & attackString(Attack attack);

        // Un alien de l'espèce spécifié
        // L'id est optionnel : il est automatiquement attribué pour être unique si
        // laissé par défaut ou négatif.
        Alien(Species species,
              int     id=-1);

        // Le constructeur par copie doit absolument changer l'id de l'alien !
        Alien(const Alien &alien);

        // L'assignation par copie doit absolument changer l'id de l'alien !
        Alien & operator=(const Alien &alien);

        // pour le polymorphisme
        virtual ~Alien();

        // vrai si en train de manger
        bool eating() const;
        // le tour auquel l'agent s'est arrété pour manger
        int eatingTurn() const;

        // énergie courante de l'alien
        int energy() const;
        // vrai si l'alien dort
        bool sleeping() const;
        // le tour auquel l'agent s'est endormi, -1 si il ne dort pas
        int sleepingTurn() const;

        // vrai si s'est déjà accouplé !
        bool hasMated() const;
        // vrai si en train de s'accoupler
        bool mating() const;
        // le tour auquel l'agent s'est arrété pour se reproduire, -1 sinon
        int matingTurn() const;

        // entier unique identifiant chaque agent
        int id() const;

        // retourne la vraie espéce de cet agent (décidée à la création)
        Species realSpecies() const;

        // la couleur choisit pour ce tour
        Alien::Color color() const;

        // l'espèce choisit pour ce tour
        Alien::Species species() const;

        // vrai si cet alien est contrôlé par un "humain" = Grutub ou Owa
        bool smart() const;

    protected:
        // appelé pour créer un clone de cet alien (servira de base pour un bébé)
        virtual ptr<Alien> clone() const = 0;

        // appelé pour savoir le déplacement que l'agent veut entreprendre
        virtual Move queryMove() = 0;

        // appelé pour savoir si l'agent veut manger la nourriture sur la case
        // sur laquelle il se trouve
        virtual bool queryEat() = 0;

        // appelé pour savoir comment l'agent veut attaquer un autre agent
        virtual Attack queryAttack(Color   alienColor,
                                   Species alienSpecies) = 0;

        // appelé pour savoir la couleur d'un agent
        virtual Color queryColor() = 0;

        // appelé pour savoir l'espèce que l'agent veut fournir aux autres
        virtual Species querySpecies() = 0;

        // appelé pour donner le tour courant
        virtual void infoTurn(int /*turn*/) { }

        // appelé pour prévenir qu'un bébé avec cet id est né !
        virtual void infoSpawn(int /*babyId*/) { }

        // appelé pour donner le statut de cet agent
        virtual void infoStatus(int /*x*/,
                                int /*y*/,
                                int /*width*/,
                                int /*height*/,
                                int /*energy*/) { }

        // appelé pour chaque voisin que l'agent voit
        virtual void infoNeighboor(int /*x*/,
                                   int /*y*/,
                                   Color /*color*/,
                                   Species /*species*/,
                                   bool /*sleeping*/,
                                   bool /*mating*/,
                                   bool /*eating*/) { }

        // appelé pour chaque nourriture que l'agent voit
        virtual void infoFood(int /*x*/,
                              int /*y*/) { }

        // appelé lorsque l'agent doit dormir (plus d'énergie)
        virtual void infoSleep() { }

        // appelé lorsque l'agent se réveille
        virtual void infoWakeup() { }

        // appelé lorsque l'agent est en train de se reproduire
        virtual void infoMate() { }

        // appelé lorsque l'agent a gagné un combat
        virtual void infoWin() { }

        // appelé lorsque l'agent a perdu un combat et va mourir
        virtual void infoLose() { }

        // appelé lorsque l'espèce de l'agent a gagné la simulation
        virtual void infoWinSimulation() { }

        // appelé lorsque l'espèce de l'agent a perdu la simulation
        virtual void infoLoseSimulation() { }

    private:
        // accessibles uniquement depuis le proxy
        void setEnergy(int energy);
        void setSleepingTurn(int turn);
        void setMatingTurn(int turn);
        void setEatingTurn(int turn);
        void setColor(Color col);
        void setSpecies(Species species);

    private:
        static std::set<int> m_ids;
        int                  m_id;
        Alien::Species       m_realSpecies;
        int                  m_energy;
        bool                 m_hasMated;
        int                  m_sleepingTurn;
        int                  m_matingTurn;
        int                  m_eatingTurn;
        Alien::Species       m_species;
        Alien::Color         m_color;

        friend class AlienProxy;
};
#endif // ALIEN_H
