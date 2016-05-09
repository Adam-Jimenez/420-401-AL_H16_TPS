#include <QtWidgets>
#include <unistd.h>
#include <stdexcept>
#include "client/clients.h"
#include "client/loop.h"

using namespace std;

int main(int   argc,
         char *argv[])
{
    QApplication app(argc, argv);

    try
    {
        string host("0.0.0.0");
        int    port = 9915;

        // TODO : ajoutez le support pour les arguments du port et l'host

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
