#include <QtWidgets>
#include <unistd.h>
#include <stdexcept>
#include "client/clients.h"
#include "client/loop.h"
#include <sstream>

using namespace std;

// blerg
inline void exitWithError(const string& str)
{
    cerr << str << endl;
    exit(1);
}

int main(int   argc,
         char *argv[])
{
    QApplication app(argc, argv);

    try
    {
        string host("0.0.0.0");
        int    port = 9915;

        // TODO : ajoutez le support pour les arguments du port et l'host
        for (int i = 1; i < argc; ++i)
        {
            string arg(argv[i]);
            if (arg == "-h")
            {
                if(i+1 < argc){
                    host = argv[i+1];
                    i++;
                }else {
                    exitWithError("Erreur, -h prend un argument en parametre! (adresse ip de l'hote)");
                }
            }
            else if (arg == "-p")
            {
                if(i+1 < argc){
                    stringstream ss(argv[i+1]);
                    if(!(ss>>port) || port<1024){
                        exitWithError("Erreur avec le parametre de port! (besoin d'un nombre superieur a 1024)");
                    }else i++;
                }else {
                    exitWithError("Erreur, -p prend un argument en parametre! (nombre superieur a 1024)");
                }
            }
        }
        ptr<Client> client = make_ptr<TcpClient>(host, port);
        ClientLoop  loop(client);
        loop.begin();
        while (!loop.done())
        {
            app.processEvents(QEventLoop::AllEvents | QEventLoop::WaitForMoreEvents);
        }
        loop.end();

        while (!loop.cleaned())
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
