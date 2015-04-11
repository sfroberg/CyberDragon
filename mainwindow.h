/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QUrl>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QFile>
#include <QNetworkAccessManager>
#include <QPointer>
#include <QStyledItemDelegate>
#include <QEvent>
#include <QApplication>
#include <QMimeDatabase>
#include <QTreeWidgetItem>
#include <QCheckBox>
#include <QLineEdit>
#include <QWebPage>

#include "version.h"

namespace Ui {
class MainWindow;
}

class   Browser;
class   general;
class   trackerblocker;
class   Cookie;
class   Proxy;
class   encryption;
class   Cache;
class   downloads;
class   notification;
class   Log;
class   Plugins;
class   ServerInfo;
class   TorrentClient;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    Browser*                        addTab(const QString& url = QLatin1String("about:blank"));
    void                            loadSettings();
    void                            saveSettings();
    Browser*                        currentTab();
    void                            loadCiphers();
public slots:
    void                            linkHovered(const QString& link,const QString& title, const QString& textContent);
    void                            loadStarted();
    void                            titleChanged(const QString& title);
    void                            closeTab(int index);
    void                            tabChanged(int index);
    void                            tabMoved(int fromIndex,int toIndex);
    /* Misc stuff */
    void                            on_actionZoom_in_triggered();
    void                            on_actionZoom_Out_triggered();
    void                            on_actionPrint_triggered();
    void                            on_actionPrint_as_PDF_triggered();
    void                            on_actionClose_Tab_triggered();
    void                            on_actionOpen_New_Tab_triggered();
    void                            on_actionAbout_CyberDragon_triggered();
    /* Download related stuff */
    const TorrentClient*            clientForRow(int row) const;
    void                            startDownload(const QUrl&);
    void                            startDownload(QNetworkReply*&);
    void                            updateProgress(int percent);
    void                            updateBytes(qint64,qint64);
    int                             rowOfClient(TorrentClient *client) const;
    void                            finishedProgress(QNetworkReply*);
    void                            failedProgress(QNetworkReply*);
    void                            startstopDownload(const QModelIndex &index);
    void                            openFile(QTreeWidgetItem * item, int column);
    void                            slot_unsupportedContent(QNetworkReply * reply);
    void                            slot_downloadRequested(const QNetworkRequest & request);
    void                            slot_FindText();
    void                            slot_FindText(const QString& text);
    void                            on_actionFindForward_triggered();
    void                            on_actionFindBackward_triggered();
    void                            on_actionFind_triggered();
private slots:
    void                            on_ReloadMap_pushButton_clicked();
private:
    friend  class                   Browser;
    friend  class                   MyCookieJar;
    friend  class                   MyNetworkAccessManager;
    friend  class                   MyWebPage;
    friend  class                   Proxy;
    friend  class                   general;
    friend  class                   downloads;
    friend  class                   trackerblocker;
    friend  class                   Cache;
    friend  class                   encryption;
    friend  class                   Plugins;
    friend  class                   ServerInfo;
    qreal                           zoomFactor;
    QString                         settingsFile;

    struct Job {
        TorrentClient *client;
        QString torrentFileName;
        QString destinationDirectory;
    };

    QList<Job>                      jobs;
    TorrentClient*                  client;
    QMimeDatabase                   mimeDatabase;
    QLineEdit*                      findBox;
    QCheckBox*                      findHighlightAll;
    QCheckBox*                      findCaseSensitive;
    QWebPage::FindFlags             findOptions;
    Ui::MainWindow*                 ui;
    /**/
    general*        __restrict__    generalTab;
    trackerblocker* __restrict__    trackerblockerTab;
    Cookie*         __restrict__    cookieTab;
    Proxy*          __restrict__    proxyTab;
    encryption*     __restrict__    encryptionTab;
    Cache*          __restrict__    cacheTab;
    downloads*      __restrict__    downloadsTab;
    notification*   __restrict__    notificationTab;
    Log*            __restrict__    logTab;
    Plugins*        __restrict__    pluginTab;
};

#endif // MAINWINDOW_H
