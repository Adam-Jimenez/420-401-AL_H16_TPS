#include <QtWidgets>
#include <iostream>
#include <sstream>
#include "alien/aliens.h"
#include "src/simulation.h"
#include "server/loop.h"
#include "server/servers.h"

using namespace std;

// faudra surement mettre sa ds un fichier de utils car le main
// de client contient la meme fonction
inline void exitWithError(const string& str)
{
    cerr << str << endl;
    exit(1);
}

int main(int   argc,
         char *argv[])
{
    QApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);

    try
    {

        string host("0.0.0.0");
        int    port             = 9915;
        bool   local            = false;
        int    numPlayers       = 1;
        int    numOfEachSpecies = 10;
        int    width            = 40;
        int    height           = 25;

        // TODO : ajoutez le support pour les différents flags
        for (int i = 1; i < argc; ++i)
        {
            string arg(argv[i]);
            if (arg == "-l")
            {
                local = true;
            }
            else if (arg == "-p") // port
            {
                if(i+1 < argc){
                    stringstream ss(argv[i+1]);
                    if(!(ss>>port) || port<1024)
                    {
                        exitWithError("Erreur avec le parametre de port! (besoin d'un nombre superieur a 1024)");
                    } else i++;
                } else {
                    exitWithError("Erreur, -p prend un argument en parametre! (nombre superieur a 1024");
                }
            }
            else if (arg == "-w") // width
            {
                if(i+1 < argc){
                    stringstream ss(argv[i+1]);
                    if(!(ss>>width) || width>50 || width<0)
                    {
                        exitWithError("Erreur avec le parametre \"width\"! (besoin d'un nombre entre 0 et 50)");
                    } else i++;
                } else {
                    exitWithError("Erreur, -w prend un argument en parametre! (nombre entre0 et 50)");
                }
            }
            else if (arg == "-h") // height
            {
                if(i+1 < argc){
                    stringstream ss(argv[i+1]);
                    if(!(ss>>height) || height>50 || height<0)
                    {
                        exitWithError("Erreur avec le parametre \"height\"! (besoin d'un nombre entre 0 et 50)");
                    } else i++;
                } else {
                    exitWithError("Erreur, -h prend un argument en parametre! (nombre entre0 et 50)");
                }
            }
            else if (arg == "-n") // numOfEachSpecies
            {
                if(i+1 < argc){
                    stringstream ss(argv[i+1]);
                    if(!(ss>>numOfEachSpecies) || numOfEachSpecies>25 || numOfEachSpecies<1)
                    {
                        exitWithError("Erreur avec le parametre \"numOfEachSpecies\"! (besoin d'un nombre entre 1 et 25)");
                    } else i++;
                } else {
                    exitWithError("Erreur, -n prend un argument en parametre! (nombre entre1 et 25)");
                }
            }
        }

        ptr<Server> server;
        if (local)
        {
            server = make_ptr<DummyServer>();
        }
        else
        {
            server     = make_ptr<TcpServer>(host, port);
            numPlayers = 2;
        }

        ptr<Simulation> sim = make_ptr<Simulation>(width, height);

        for (int j = 0; j < numOfEachSpecies; ++j)
        {
            ptr<Alien> alien = make_ptr<AlienUqomua>();
            sim->addLocalAlien(alien);
        }

        for (int j = 0; j < numOfEachSpecies; ++j)
        {
            ptr<Alien> alien = make_ptr<AlienOg>();
            sim->addLocalAlien(alien);
        }

        for (int j = 0; j < numOfEachSpecies; ++j)
        {
            ptr<Alien> alien = make_ptr<AlienYuhq>();
            sim->addLocalAlien(alien);
        }

        for (int j = 0; j < numOfEachSpecies; ++j)
        {
            ptr<Alien> alien = make_ptr<AlienEpoe>();
            sim->addLocalAlien(alien);
        }

        for (int j = 0; j < numOfEachSpecies; ++j)
        {
            for (int i = 0; i < numPlayers; ++i)
            {
                ptr<Alien> alien = make_ptr<SmartAlien>(
                            static_cast<Alien::Species>(Alien::Grutub + i));
                if (!local)
                {
                    sim->addRemoteAlien(server, i, alien);
                }
                else
                {
                    sim->addLocalAlien(alien);
                }
            }
        }

        ptr<ServerLoop> loop = make_ptr<ServerLoop>(sim, server);
        loop->begin();
        while (!loop->done())
        {
            app.processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);
        }
        loop->end();
        while (!loop->cleaned())
        {
            // processe les derniers événements qui pourraient rester dans la queue !
            app.processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);
        }
    }
    catch (runtime_error &e)
    {
        cerr << "Une exception s'est produite!" << endl;
        cerr << e.what() << endl;

        return 1;
    }


    return 0;
}
