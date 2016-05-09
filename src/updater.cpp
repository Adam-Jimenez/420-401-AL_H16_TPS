#include "src/simulation.h"
#include "src/updater.h"
#include <QtWidgets>

Updater::Updater(ptr<Simulation> sim,
                 ptr<Gui>        gui,
                 QObject        *parent) :
    QObject(parent),
    m_sim(sim)
{
    connect(gui.get(), SIGNAL(pauseClicked()),
            this, SLOT(update()));
    connect(gui.get(), SIGNAL(continueClicked()),
            this, SLOT(update()));
    connect(gui.get(), SIGNAL(turnClicked()),
            this, SLOT(update()));
    connect(gui.get(), SIGNAL(tickClicked()),
            this, SLOT(update()));
}

void Updater::update()
{
    if (m_sim->updating())
    {
        // réessaye jusqu'a ce que ce soit correct
        QTimer::singleShot(50, this, SLOT(update()));
    }
    else
    {
        m_timer.restart();
        m_sim->update();
    }
}

void Updater::scheduleNextUpdate(int delay)
{
    QTimer::singleShot(qMax(delay - m_timer.elapsed(), qint64(0)), this, SLOT(update()));
}
