#include <QtWidgets>
#include <QtSvg>
#include <iostream>
#include "src/gui.h"
#include "src/simulation.h"
#include "alien/alien.h"

class TooltipTextItem : public QGraphicsTextItem
{
    Q_OBJECT
    // Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)

    public:
        TooltipTextItem(ptr<QGraphicsView> view) :
            view(view)/*, anim(this, "opacity")*/
        {
            setZValue(1);
            hide();
        }

        void setText(ptr<Alien> alien)
        {
            QString color;
            switch (alien->realSpecies())
            {
                case Alien::Grutub: color = "#F7A8C4"; break;
                case Alien::Owa:    color = "#59BFE0"; break;
                default:            color = "white"; break;
            }

            QString msg = trUtf8("<body bgcolor='%8'>Id: %1<br/>"
                                 "Espèce réelle: %2<br/>"
                                 "Espèce courante: %3<br/>"
                                 "Énergie: %4<br/>"
                                 "Dort: %5<br/>"
                                 "Mange: %6<br/>"
                                 "Se reproduit: %7</body>")
                          .arg(alien->id())
                          .arg(QString::fromStdString(Alien::speciesString(alien->
                                                                           realSpecies())))
                          .arg(QString::fromStdString(Alien::speciesString(
                                                          alien->species())))
                          .arg(alien->energy())
                          .arg(alien->sleeping() ? "oui" : "non")
                          .arg(alien->eating() ? "oui" : "non")
                          .arg(alien->mating() ? "oui" : "non")
                          .arg(color);

            setHtml(msg);
        }

        void show()
        {
            if (!isVisible())
            {
                // anim.stop();
                // setOpacity(1.0);
                setVisible(true);
                view->viewport()->update();
            }
        }

        /*
         *   void setOpacity(qreal opacity)
         *   {
         *    QGraphicsItem::setOpacity(opacity);
         *    view->viewport()->update();
         *   }
         */

        void hide()
        {
            /*
             *   if (opacity() >= 1.0)
             *   {
             *    anim.setDuration(1000);
             *    anim.setStartValue(0.999);
             *    anim.setEndValue(0.0);
             *    anim.start();
             *    setOpacity(0.0);
             *   }
             */
            if (isVisible())
            {
                setVisible(false);
                view->viewport()->update();
            }
        }

        ptr<QGraphicsView> view;
        // QPropertyAnimation anim;
};

Gui::Gui(int width,
         int height) :
    m_updateType(NONE),
    m_updateTime(100),
    m_updateDebug(false),
    m_width(width),
    m_height(height)
{
    setupImages();
    setWindowTitle(trUtf8("Simulation d'agents autonomes intelligents"));

    m_cellSize = qMin(m_bgSprite.width() / (width + 2),
                      m_bgSprite.height() / (height + 2));

    m_mLay = make_ptr<QVBoxLayout>(this);
    {
        m_scene = make_ptr<QGraphicsScene>(this);
        // m_scene->setSceneRect(0, 0, width*m_cellSize, height*m_cellSize);
        int H = height * m_cellSize;
        int W = width * m_cellSize;
        m_scene->setSceneRect(0, -0.1 * H, W, 1.1 * H);
        m_scene->installEventFilter(this);

        m_bgItem = make_ptr<QGraphicsPixmapItem>();
        m_bgItem->setPixmap(m_bgSprite);
        m_bgItem->moveBy(0.5 * (W - m_bgSprite.width()),
                         0.5 * (H - m_bgSprite.height()));
        m_scene->addItem(m_bgItem.get());

        m_stats1Item = make_ptr<QGraphicsTextItem>();
        m_stats2Item = make_ptr<QGraphicsTextItem>();
        std::vector<int> counts(6, 999);
        setStats(counts, 999, 9999);
        QRectF r1 = m_stats1Item->sceneBoundingRect();
        m_stats1Item->setScale(0.1 * H / r1.height());
        m_stats1Item->setPos(0.1 * W, -0.1 * H);
        QRectF r2 = m_stats2Item->sceneBoundingRect();
        m_stats2Item->setScale(0.1 * H / r2.height());
        m_stats2Item->setPos(W - 0.1 * W - m_stats2Item->sceneBoundingRect().width(),
                             /*m_stats1Item->sceneBoundingRect().width()*1.1,*/
                             -0.1 * H);
        m_scene->addItem(m_stats1Item.get());
        m_scene->addItem(m_stats2Item.get());

        m_view = make_ptr<QGraphicsView>();
        m_view->setWindowTitle("Simulation d'aliens");
        m_view->setScene(m_scene.get());
        // m_view->setRenderHints(QPainter::Antialiasing|QPainter::TextAntialiasing);
        m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        m_view->setBackgroundBrush(Qt::black);

        ptr<TooltipTextItem> titem = make_ptr<TooltipTextItem>(m_view);
        m_tooltipItem = titem;
        m_scene->addItem(titem.get());

        m_mLay->addWidget(m_view.get());
    }

    {
        m_play = make_ptr<QPushButton>();
        m_play->setIcon(QIcon(":/rc/icons/play.png"));
#ifndef NACL
        m_play->setToolTip(trUtf8("Reprendre la simulation"));
#endif
        m_play->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        m_play->setCheckable(true);
        connect(m_play.get(), SIGNAL(toggled(bool)), this, SLOT(playOrPause()));

        m_speed = make_ptr<QSlider>(Qt::Horizontal);
        m_speed->setRange(0, 1000);
        m_speed->setValue(900);
        m_speed->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(m_speed.get(), SIGNAL(valueChanged(int)), this, SLOT(setSpeed(int)));

        m_tick = make_ptr<QPushButton>();
#ifndef NACL
        m_tick->setToolTip(trUtf8("Simuler le prochain alien"));
#endif
        m_tick->setIcon(QIcon(":/rc/icons/skip_forward.png"));
        m_tick->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(m_tick.get(), SIGNAL(clicked()), this, SLOT(tick()));

        m_turn = make_ptr<QPushButton>();
#ifndef NACL
        m_turn->setToolTip(trUtf8("Simuler un tour de tous les aliens restants"));
#endif
        m_turn->setIcon(QIcon(":/rc/icons/fast_forward.png"));
        m_turn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        connect(m_turn.get(), SIGNAL(clicked()), this, SLOT(turn()));

        m_debugCb = make_ptr<QCheckBox>(trUtf8("Activer la trace"));
        connect(m_debugCb.get(), SIGNAL(toggled(bool)), this, SLOT(setDebug(bool)));

        m_hLay = make_ptr<QHBoxLayout>();
        m_hLay->addWidget(m_play.get());
        m_hLay->addWidget(m_speed.get());
        m_hLay->addSpacing(30);
        m_hLay->addWidget(m_tick.get());
        m_hLay->addWidget(m_turn.get());
        m_hLay->addStretch();
        m_hLay->addWidget(m_debugCb.get());

        m_debug = make_ptr<QTextEdit>();
        m_debug->setReadOnly(true);
        m_debug->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_debug->setMinimumHeight(200);
        m_debug->hide();

        m_groupOptions = make_ptr<QGroupBox>(trUtf8("Options"));
        m_vLay         = make_ptr<QVBoxLayout>(m_groupOptions.get());
        m_vLay->addLayout(m_hLay.get());
        m_vLay->addWidget(m_debug.get());
        m_mLay->addWidget(m_groupOptions.get());
    }

    m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_view->setViewportUpdateMode(QGraphicsView::NoViewportUpdate);
    m_view->show();

    resize(qApp->primaryScreen()->size().width() * 0.75,
           qApp->primaryScreen()->size().height() * 0.75);

    // repaint le GUI une fois affiché (pour nacl)
    QTimer::singleShot(200, this, SLOT(repaint()));
    // startTimer(1000);
}

void Gui::addFood(int x,
                  int y)
{
    ptr<QGraphicsSvgItem> item = make_ptr<QGraphicsSvgItem>();
    item->setSharedRenderer(m_foodSpriteItem->renderer());
    item->setCacheMode(QGraphicsItem::NoCache);

    QSizeF size  = item->boundingRect().size();
    qreal  scale = m_cellSize / (size.width() * 2);
    item->setScale(scale);
    item->setPos((x + 0.25) * m_cellSize, (y + 0.25) * m_cellSize);

    m_foodsToItems[y * m_width + x] = item;

    m_scene->addItem(item.get());
}

void Gui::addAlien(ptr<Alien> alien,
                   int        x,
                   int        y)
{
    qreal  scale;
    QSizeF size;

    ptr<QGraphicsSvgItem> item = make_ptr<QGraphicsSvgItem>();
    item->setSharedRenderer(m_alienSpritesItem[qMakePair(alien->color(),
                                                         alien->species())]->renderer());
    item->setCacheMode(QGraphicsItem::NoCache);
    size  = item->boundingRect().size();
    scale = m_cellSize / size.width();
    item->setScale(scale);
    item->setPos(x * m_cellSize, y * m_cellSize);
    item->setFlag(QGraphicsItem::ItemIsSelectable);

    m_aliensToItems[alien] = item;
    m_itemsToAliens[item]  = alien;

    m_scene->addItem(item.get());
}

void Gui::removeFood(int x,
                     int y)
{
    m_foodsToItems.remove(y * m_width + x);
}

void Gui::removeAlien(ptr<Alien> alien)
{
    ptr<QGraphicsSvgItem> item = m_aliensToItems[alien];
    m_aliensToItems.remove(alien);
    m_itemsToAliens.remove(item);
}

void Gui::moveAlien(ptr<Alien> alien,
                    int        x,
                    int        y)
{
    removeAlien(alien);
    addAlien(alien, x, y);
}

Gui::UpdateType Gui::updateType() const
{
    return m_updateType;
}

void Gui::setUpdateType(Gui::UpdateType type)
{
    m_updateType = type;
}

int Gui::updateTime() const
{
    return m_updateTime;
}

bool Gui::debug() const
{
    return m_updateDebug;
}

void Gui::fitAndRefresh()
{
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);
    refresh();
}

void Gui::refresh()
{
    m_view->viewport()->update();
}

void Gui::setupImages()
{
    for (int j = 0; j < Simulation::MaxNumSpecies; ++j)
    {
        for (int i = 0; i < Alien::Orange + 1; ++i)
        {
            QPair<Alien::Color, Alien::Species> pair(static_cast<Alien::Color>(i),
                                                     static_cast<Alien::Species>(j));
            QString path = QString(":/rc/svg/%1.svg").arg(
                j * (Alien::Orange + 1) + i, 2, 10, QChar('0'));
            m_alienSpritesItem[pair] = make_ptr<QGraphicsSvgItem>(path);
        }
    }
    m_foodSpriteItem = make_ptr<QGraphicsSvgItem>(QString(":/rc/svg/pop.svg"));
    m_bgSprite       = QPixmap(QString(":/rc/img/bg.jpg"));
}

void Gui::playOrPause()
{
    if (m_play->isChecked())
    {
        m_play->setIcon(QIcon(":/rc/icons/pause.png"));
#ifndef NACL
        m_play->setToolTip(trUtf8("Mettre la simulation en pause"));
#endif
        m_updateType = ALL;
        m_speed->setEnabled(false);
        m_tick->setEnabled(false);
        m_turn->setEnabled(false);
        emit continueClicked();
    }
    else
    {
        m_play->setIcon(QIcon(":/rc/icons/play.png"));
#ifndef NACL
        m_play->setToolTip(trUtf8("Reprendre la simulation"));
#endif
        m_updateType = NONE;
        m_speed->setEnabled(true);
        m_tick->setEnabled(true);
        m_turn->setEnabled(true);
        emit pauseClicked();
    }

    // repaint le GUI une fois affiché (pour nacl)
    QTimer::singleShot(200, this, SLOT(repaint()));
}

void Gui::tick()
{
    m_updateType = TICK;
    emit tickClicked();

    // repaint le GUI une fois affiché (pour nacl)
    QTimer::singleShot(200, this, SLOT(repaint()));
}

void Gui::turn()
{
    m_updateType = TURN;
    emit turnClicked();

    // repaint le GUI une fois affiché (pour nacl)
    QTimer::singleShot(200, this, SLOT(repaint()));
}

void Gui::setSpeed(int value)
{
    m_updateTime = 1000 - value;
}

void Gui::setDebug(bool val)
{
    m_updateDebug = val;
    m_debug->setVisible(val);
    // m_debug->clear();
    repaint();
    QTimer::singleShot(200, this, SLOT(fitAndRefresh()));
}

void Gui::appendDebug(const std::string &debug)
{
    m_debug->append(QString::fromStdString(debug));
}

void Gui::setStats(const std::vector<int> &counts,
                   int                     turns,
                   int                     foods)
{
    QString msg;
    msg  = "<body bgcolor='black'><font color='white'>";
    msg += "<table><tr><td width='70'>Tour</td>"
           "<td width='70'>Nourriture</td></tr>"
           "<tr><td width='70'><b>%1</b></td>"
           "<td width='70'><b>%2</b></td></tr></table>";
    msg += "</font></body>";
    msg  = msg.arg(turns).arg(foods);
    m_stats1Item->setHtml(msg);

    msg  = "<body bgcolor='black'><font color='white'>";
    msg += "<table><tr>";
    for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
    {
        Alien::Species sp = static_cast<Alien::Species>(i);
        msg += QString("<td align='center' width='50'>%1</td>")
               .arg(QString::fromStdString(Alien::speciesString(sp)));
    }
    msg += "</tr>";
    msg += "<tr>";
    for (int i = Alien::Uqomua; i <= Alien::Owa; ++i)
    {
        msg += QString("<td align='center'><b>%1</b></td>").arg(counts[i]);
    }
    msg += "</tr></table>";
    msg += "</font></body>";
    m_stats2Item->setHtml(msg);
}

void Gui::showMessage(const std::string &msg)
{
    QMessageBox::information(this,
                             trUtf8("Simulation d'aliens intelligents"),
                             QString::fromStdString(msg));
}

bool Gui::eventFilter(QObject *obj,
                      QEvent  *event)
{
    if (obj == m_scene.get())
    {
        if (event->type() == QEvent::GraphicsSceneMouseRelease)
        {
            QGraphicsSceneMouseEvent *mev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            QList<QGraphicsItem *> items = m_scene->selectedItems();
            if (!items.empty())
            {
                QGraphicsSvgItem *item = qgraphicsitem_cast<QGraphicsSvgItem *>(items[0]);
                if (item)
                {
                    ptr<Alien> alien =
                        m_itemsToAliens.value(make_non_owning_ptr<QGraphicsSvgItem>(item));
                    if (!alien.empty())
                    {
                        TooltipTextItem *titem =
                            dynamic_cast<TooltipTextItem *>(m_tooltipItem.get());
                        titem->setText(alien);
                        QRectF r  = item->sceneBoundingRect();
                        QRectF rt = titem->sceneBoundingRect();
                        double x  = (r.x() < m_scene->sceneRect().width() / 2 ?
                                     r.left() : r.right() - rt.width());
                        double y = (r.y() < m_scene->sceneRect().height() / 2 ?
                                    r.bottom() : r.top() - rt.height());
                        titem->setPos(x, y);
                        titem->show();
                        m_lastTooltipPos = mev->scenePos();
                    }
                }
            }
        }
        else if (event->type() == QEvent::GraphicsSceneMouseMove)
        {
            QGraphicsSceneMouseEvent *mev =
                static_cast<QGraphicsSceneMouseEvent *>(event);
            QPointF p  = mev->scenePos();
            QPointF p2 = m_lastTooltipPos;
            double  dx = p.x() - p2.x();
            double  dy = p.y() - p2.y();
            if (dx * dx + dy * dy > 100 * 100)
            {
                TooltipTextItem *titem =
                    dynamic_cast<TooltipTextItem *>(m_tooltipItem.get());
                titem->hide();
            }
        }
    }

    return QObject::eventFilter(obj, event);
}

void Gui::timerEvent(QTimerEvent *)
{
    repaint();
}

void Gui::showEvent(QShowEvent *event)
{
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);
    QWidget::showEvent(event);
}

void Gui::resizeEvent(QResizeEvent *event)
{
    m_view->fitInView(m_view->scene()->sceneRect(), Qt::KeepAspectRatio);
    QWidget::resizeEvent(event);
}

#include "gui.moc"
