/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef PROXY_H
#define PROXY_H

#include <QNetworkProxy>
#include <QTimer>
#include <QWebPage>
#include <QButtonGroup>
#include <QAction>
#include <QTreeWidgetItem>
#include <QThread>

#include "worker.h"

#ifdef  Q_OS_WIN32
#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <icmpapi.h>
#endif

#include <zlib.h>
#include <GeoIP.h>


namespace Ui {
class Proxy;
}

class MainWindow;

class Proxy : public QWidget
{
    Q_OBJECT
public:
    static quint32  proxy_list_counter;
    static quint32  total_number_of_proxies;
    static quint32  total_number_of_working_proxies;
    static  quint32 NUMBER_OF_PROXIES;
    static  quint32 OFFLINE_PROXIES;
    static qint64   NUMBER_OF_THREADS;
    static quint32  proxy_checker_timeout;
    static quint32  NUMBER_OF_PROXY_SITES;

    friend class    MyNetworkAccessManager;

    explicit Proxy(MainWindow *m, QWidget *parent = 0);
    ~Proxy();
    MainWindow*         getMainWindow();
    void                loadSettings();
    void                saveSettings();
signals:
    void                readyForProxyChecking();
    void                allProxiesFinished();
    void                abortConnections();
public slots:
    void                on_ClearProxy_pushButton_clicked();
    void                randomProxy();
    void                proxyHoppingbuttonClicked ( int id );
    void                coolproxy(bool);
    void                samair(bool);
    void                freeproxylist(bool);
    void                free_proxy_list(bool);
    void                xroxy(bool);
    void                xroxy1(bool);
    void                xroxy2(bool);
    void                proxy_list(bool);
    void                slot_allProxiesFinished();
    void                dataReady(const QStringList& data,const QColor& color);
    void                proxyListReady();
    void                removeProxy();
private slots:
    void                on_AddBlacklistedProxies_pushButton_clicked();
    void                on_GetProxies_pushButton_clicked();
    void                on_ProxyFetcherChecker_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void                on_SetProxy_pushButton_clicked();
    void                on_ExportProxies_pushButton_clicked();
    void                on_ImportProxies_pushButton_clicked();
    void                on_CheckProxies_pushButton_clicked();
    void                on_stopChecking_pushButton_clicked();
    void                on_RemoveBlacklistedProxies_pushButton_clicked();
    void                on_ProxyHopping_pushButton_clicked();
    void                on_removeNotWorkingProxies_pushButton_clicked();
private:
    friend class        Browser;
    QString             settingsFile;
    MainWindow*         mainWindow;
    QTimer              proxyHoppingTimer;
    QNetworkProxy       proxy;          // Current proxy used
    QSet<QString>       proxyList;
    QList<QString>      proxies;
    QWebPage*           proxy_freeproxylists;
    QWebPage*           proxy_free_proxy_list;
    QWebPage*           proxy_xroxy;
    QWebPage*           proxy_xroxy1;
    QWebPage*           proxy_xroxy2;
    QWebPage*           proxy_samair;
    QWebPage*           proxy_coolproxy;
    QWebPage*           proxy_proxy_list;
    GeoIP*              gi;
    QButtonGroup*       proxyHopping;
    QAction*            action_removeProxy;
    Worker*             worker;
    QThread*            workerThread;
    Ui::Proxy*          ui;
};

namespace RandomProxyHopping {
    enum  {
        NoProxyHopping,
        ProxyHoppingTimed,
        ProxyHoppingRequest
    };
}

#endif // PROXY_H
