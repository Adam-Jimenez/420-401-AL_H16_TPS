#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include <QElapsedTimer>
#include "src/ptr.h"
#include "src/gui.h"
#include "src/simulation.h"

class Simulation;
class Updater : public QObject
{
    Q_OBJECT

    public:
        Updater(ptr<Simulation> game,
                ptr<Gui>        gui,
                QObject        *parent=0);

    public slots:
        void update();
        void scheduleNextUpdate(int delay);

    private:
        ptr<Simulation> m_sim;
        QElapsedTimer   m_timer;
};
#endif // UPDATER_H
