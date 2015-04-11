/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef SERVERINFO_H
#define SERVERINFO_H

#include <QWidget>
#include <QHostInfo>

#include <zlib.h>
#include <GeoIP.h>
#include <GeoIPCity.h>

namespace Ui {
class ServerInfo;
}

class MainWindow;

class ServerInfo : public QWidget
{
    Q_OBJECT
public:
    explicit ServerInfo(MainWindow* m, QWidget *parent = 0);
    ~ServerInfo();
public slots:
    void                lookUp(const QHostInfo &host);
    void                reverselookUp(const QHostInfo &host);
private slots:
    void                on_webView_loadFinished(bool arg1);
    void                loadGoogleMaps();
private:
    Ui::ServerInfo*     ui;
    friend class        MainWindow;
    MainWindow*         mainWindow;
    GeoIP*              gi;
    QString             url;
    float               latitude;
    float               longitude;
    QString             settingsFile;
};

#endif // SERVERINFO_H
