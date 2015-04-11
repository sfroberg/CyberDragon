/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "notification.h"
#include "ui_notification.h"

#include <QDebug>
#include <QSettings>

/* Notification class. Supports stacking, timed, freely movable (and remembers it's last place) notification
 * "sticky" notes.
 *
 * This module needs some loving.
 * If all else fails, try to find some Notification API (at least Nokia had one) and rewrite the whole damn thing.
 * As a last resort, use Qt systray class if things can't be improved here */

notification::notification(QWidget *parent) :
    QWidget(parent),settingsFile(QLatin1String("config.ini")),
    ui(new Ui::notification)
{
    ui->setupUi(this);
    this->loadSettings();
}

notification::~notification()
{
    this->saveSettings();
    delete ui;
}

void
notification::loadSettings()
{
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("notify"));
    this->ui->notifications_checkBox->setChecked(settings.value(QLatin1String("notify_enabled")).toBool());
    this->ui->notifyTimeout_spinBox->setValue(settings.value(QLatin1String("notify_timeout")).toUInt());
    this->ui->notifyDownloadStart_checkBox->setChecked(settings.value(QLatin1String("notify_download_start")).toBool());
    this->ui->notifyDownloadError_checkBox->setChecked(settings.value(QLatin1String("notify_download_error")).toBool());
    this->ui->notifyDownloadComplete_checkBox->setChecked(settings.value(QLatin1String("notify_download_finish")).toBool());

    if(!settings.contains(QLatin1String("notify_x")))
        Notify::oldPos.setX(0);
    else
        Notify::oldPos.setX(settings.value(QLatin1String("notify_x")).toUInt());

    if(!settings.contains(QLatin1String("notify_y")))
        Notify::oldPos.setY(0);
    else
        Notify::oldPos.setY(settings.value(QLatin1String("notify_y")).toUInt());
    settings.endGroup();

}

void
notification::saveSettings()
{
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("notify"));
    settings.setValue(QLatin1String("notify_enabled"),this->ui->notifications_checkBox->isChecked());
    settings.setValue(QLatin1String("notify_timeout"),this->ui->notifyTimeout_spinBox->value());
    settings.setValue(QLatin1String("notify_download_start"),this->ui->notifyDownloadStart_checkBox->isChecked());
    settings.setValue(QLatin1String("notify_download_error"),this->ui->notifyDownloadError_checkBox->isChecked());
    settings.setValue(QLatin1String("notify_download_finish"),this->ui->notifyDownloadComplete_checkBox->isChecked());
    settings.setValue(QLatin1String("notify_x"),Notify::oldPos.x());
    settings.setValue(QLatin1String("notify_y"),Notify::oldPos.y());
    settings.endGroup();
}



QPoint 		Notify::oldPos;
QPoint      Notify::relPos;
int         Notify::width = 300;
int         Notify::height = 120;
int         Notify::TIMEOUT = 10;


Notify::Notify(QWidget *parent) :
    QDialog(parent)
{
    this->setup();
}

Notify::Notify(const QString& title,const QString& info,int timeout,QWidget *parent) :
    QDialog(parent),title(title),info(info),timeout(timeout),current_timeout(timeout),show_timeout(true)
{
    this->setup();

    move(oldPos.x(),oldPos.y());
#ifndef NDEBUG
    qDebug() << "NOTIFY CREATED ON POSITION " << oldPos;
#endif
}

Notify::~Notify()
{
    if(this->timeout != 0)
        basicTimer.stop();
}


void    Notify::setup()
{
    if (this->objectName().isEmpty())
        this->setObjectName(QStringLiteral("Notify"));

        this->setModal(false);

        this->resize(300, 120);
        QSizePolicy sizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(this->sizePolicy().hasHeightForWidth());
        this->setSizePolicy(sizePolicy);
        QFont font;
        font.setStyleStrategy(QFont::PreferAntialias);
        this->setFont(font);
        this->setStyleSheet("QDialog#Notify {background-color: rgb(255, 255, 153);}");
        close_pushButton = new QPushButton(this);
        close_pushButton->setObjectName(QStringLiteral("close_pushButton"));
        close_pushButton->setGeometry(QRect(270, 4, 20, 20));
        QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(close_pushButton->sizePolicy().hasHeightForWidth());
        close_pushButton->setSizePolicy(sizePolicy1);
        close_pushButton->setStyleSheet(QLatin1String("QPushButton {\n"
"background-color: rgba(255, 255, 255, 10)\n"
"}"));
        QIcon icon;
        icon.addFile(QStringLiteral(":/icons/16x16/dialog-close-2.png"), QSize(), QIcon::Normal, QIcon::Off);
        close_pushButton->setIcon(icon);
        close_pushButton->setFlat(true);
        title_label = new QLabel(this);
        title_label->setObjectName(QStringLiteral("title_label"));
        title_label->setText(title + "\n");
        title_label->setGeometry(QRect(30, 4, 211, 32));
        sizePolicy1.setHeightForWidth(title_label->sizePolicy().hasHeightForWidth());
        title_label->setSizePolicy(sizePolicy1);
        QFont font1;
        font1.setPointSize(8);
        font1.setBold(true);
        font1.setWeight(75);
        font1.setStyleStrategy(QFont::PreferAntialias);
        title_label->setFont(font1);
        title_label->setAlignment(Qt::AlignCenter);
        info_label = new QLabel(this);
        info_label->setObjectName(QStringLiteral("info_label"));
        info_label->setGeometry(QRect(16, 40, 268, 56));
        info_label->setText(info);
        info_label->setWordWrap(true);
        sizePolicy1.setHeightForWidth(info_label->sizePolicy().hasHeightForWidth());
        info_label->setSizePolicy(sizePolicy1);
        info_label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);

        QObject::connect(close_pushButton, SIGNAL(clicked()), this, SLOT(close()));
        QMetaObject::connectSlotsByName(this);

        close_pushButton->setToolTip("Close");
        close_pushButton->setText(QString());


    this->setWindowFlags(Qt::CustomizeWindowHint |Qt::FramelessWindowHint);

   if(this->timeout != 0)
        this->basicTimer.start(1000,this);   // 1-second timer

    QPixmap  pixmap(":/icons/notify.png");
    this->setMask(pixmap.mask());
}

void Notify::mousePressEvent(QMouseEvent *evt)
{
    if(this->timeout != 0)
        this->basicTimer.stop();

    this->title_label->setText(title);
    relPos = evt->pos();
    qDebug() << "Mousepress " << oldPos;
    qDebug() << "oldPos " << oldPos;
    qDebug() << "=========================================================";
    qDebug() << "Widget coordinates " << mapFromGlobal(evt->globalPos());
    qDebug() << "Widget coordinates2 " << mapFrom(this,evt->globalPos());
    qDebug() << "Widget coordinates3 " << mapFromParent(evt->globalPos());
    qDebug() << "=========================================================";
    qDebug() << "Widget coordinates4 " << mapFromGlobal(oldPos);
    qDebug() << "Widget coordinates5 " << mapFrom(this,oldPos);
    qDebug() << "Widget coordinates6 " << mapFromParent(oldPos);
    qDebug() << "=========================================================";
    qDebug() << "Widget coordinates7 " << mapFromGlobal(evt->pos());
    qDebug() << "Widget coordinates8 " << mapFrom(this,evt->pos());
    qDebug() << "Widget coordinates9 " << mapFromParent(evt->pos());

    qDebug() <<"evt-globalPos() " << evt->globalPos();
    qDebug() <<"evt-pos() " << evt->pos();

    qDebug() << "=========================================================";
    qDebug() << "screenPos " << evt->screenPos();
    qDebug() << "windowPos " << evt->windowPos();
    qDebug() << "localPos " << evt->localPos();
    qDebug() << "x " << evt->x();
    qDebug() << "y " << evt->y();
    qDebug() << "=========================================================";
}


void    Notify::mouseReleaseEvent(QMouseEvent * evt)
{
}

void Notify::mouseMoveEvent(QMouseEvent *evt)
{

    const QPoint delta = evt->globalPos() - oldPos;

    qDebug() << "Delta " << delta;
    qDebug() << x() + delta.x() << ", " << y() + delta.y();
    qDebug() << "OldPos - Delta " <<(oldPos.x() + delta.x()) << "," << (oldPos.y() + delta.y());

    oldPos.setX((oldPos.x() + delta.x()));
    oldPos.setY((oldPos.y()) + delta.y());

    oldPos -= relPos;

    move(oldPos);
}



void Notify::timerEvent(QTimerEvent *event) {

    QString text = this->title;

    if(this->timeout != 0) {
        if(show_timeout) {
            text += "\n(closing in: " + QString::number(this->timeout) + ")";
        } else {
            text += "\n";
        }
    }
    this->title_label->setText(text);
    if(this->timeout != 0)
    if(--this->timeout == 0) {
        this->basicTimer.stop();
        this->close();
    }
}

void    Notify::setTitle(const QString& title)
{
    this->title = title;
}

void    Notify::setInfo(const QString& info)
{
    this->info = info;
}

void    Notify::setTimeout(int timeout)
{
    this->current_timeout = this->timeout = timeout;
}

