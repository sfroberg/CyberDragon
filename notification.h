/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef NOTIFICATION_H
#define NOTIFICATION_H

#include <QWidget>


/* Trivial custom notification system.
 * TODO: Would Qt systray class be more elegant? */

namespace Ui {
class notification;
}

class notification : public QWidget
{
    Q_OBJECT
public:
    explicit notification(QWidget *parent = 0);
    ~notification();
    void    loadSettings();
    void    saveSettings();
private:
    friend class MainWindow;
    friend class proxy;
    friend class Proxy;
    friend class trackerblocker;
    friend class MyNetworkAccessManager;
    QString settingsFile;
    Ui::notification *ui;
};


#include <QDialog>
#include <QPoint>
#include <QPushButton>
#include <QLabel>
#include <QBitmap>
#include <QMouseEvent>
#include <QBasicTimer>

class Notify : public QDialog
{
    Q_OBJECT
public:
    static QPoint 		oldPos;
    static QPoint       relPos;
    static  int         width;
    static  int         height;
    static  int         TIMEOUT;

    static void setPos(int x,int y)
    {
        oldPos.setX(x);
        oldPos.setY(y);
    }

    explicit Notify(QWidget *parent = 0);
    explicit Notify(const QString&,const QString&,int timeout = Notify::TIMEOUT,QWidget *parent = 0);
    ~Notify();
    inline void    setTitle(const QString&);
    inline void    setInfo(const QString&);
    inline void    setTimeout(int);
    void    setTimeoutState(bool state)
    {
        this->show_timeout = state;
    }
protected:
    void mousePressEvent(QMouseEvent *evt);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *evt);
    void timerEvent(QTimerEvent *event);
    inline void    setup();
private:
    QPushButton *close_pushButton;
    QLabel      *title_label;
    QLabel      *info_label;
    QString     title;
    QString     info;
    int         timer_id;
    int         timeout;
    int         current_timeout;
    bool        show_timeout;
    QBasicTimer basicTimer;
};

#endif // NOTIFICATION_H
