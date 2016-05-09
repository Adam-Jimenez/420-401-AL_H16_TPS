#ifndef GUI_H
#define GUI_H

#include <QtWidgets> // pas cool, mais pour que ptr<> fonctionne !
#include <QtSvg>     // pas cool, mais pour que ptr<> fonctionne !
#include "src/ptr.h"
#include "alien/alien.h"

class TooltipTextItem;
class Gui : public QWidget
{
    Q_OBJECT

    public:
        Gui(int width,
            int height);

        void addFood(int x,
                     int y);
        void addAlien(ptr<Alien> alien,
                      int        x,
                      int        y);

        void removeFood(int x,
                        int y);
        void removeAlien(ptr<Alien> alien);

        void moveAlien(ptr<Alien> alien,
                       int        x,
                       int        y);

        enum UpdateType { NONE, TICK, TURN, ALL };
        UpdateType updateType() const;
        void setUpdateType(UpdateType type);
        int updateTime() const;
        bool debug() const;

        void refresh();

    private:
        void setupImages();

    public slots:
        void playOrPause();
        void tick();
        void turn();
        void setSpeed(int);
        void setDebug(bool);
        void appendDebug(const std::string &debug);
        void setStats(const std::vector<int> &counts,
                      int                     turns,
                      int                     foods);
        void showMessage(const std::string &msg);
        void fitAndRefresh();

    protected:
        void showEvent(QShowEvent *);
        void resizeEvent(QResizeEvent *);
        bool eventFilter(QObject *,
                         QEvent *);
        void timerEvent(QTimerEvent *);

    signals:
        void pauseClicked();
        void continueClicked();
        void tickClicked();
        void turnClicked();

    private:
        // l'ordre est important ici .. Qt va essayer de détruire ces objets alors il faut
        // que le dtor de ptr passe en premier !
        ptr<QGraphicsView>  m_view;
        ptr<QGraphicsScene> m_scene;

        ptr<QVBoxLayout> m_mLay;

        ptr<QGroupBox>   m_groupOptions;
        ptr<QVBoxLayout> m_vLay;

        ptr<QPushButton> m_play;
        ptr<QPushButton> m_tick;
        ptr<QPushButton> m_turn;
        ptr<QSlider>     m_speed;
        ptr<QTextEdit>   m_debug;
        ptr<QCheckBox>   m_debugCb;
        ptr<QHBoxLayout> m_hLay;

        UpdateType m_updateType;
        int        m_updateTime;
        bool       m_updateDebug;

        QMap< ptr<Alien>, ptr<QGraphicsSvgItem> > m_aliensToItems;
        QMap< ptr<QGraphicsSvgItem>, ptr<Alien> > m_itemsToAliens;

        ptr<QGraphicsItem>                m_tooltipItem;
        QMap<int, ptr<QGraphicsSvgItem> > m_foodsToItems;

        QMap< QPair<Alien::Color, Alien::Species>,
              ptr<QGraphicsSvgItem> > m_alienSpritesItem;
        ptr<QGraphicsSvgItem>
                                 m_foodSpriteItem;
        ptr<QGraphicsPixmapItem> m_bgItem;
        ptr<QGraphicsTextItem>   m_stats1Item;
        ptr<QGraphicsTextItem>   m_stats2Item;
        QPixmap                  m_bgSprite;
        QPointF
            m_lastTooltipPos;

        int m_width;
        int m_height;
        int m_cellSize;
};
#endif // GUI_H
