/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MYNETWORKACCESSMANAGER_H
#define MYNETWORKACCESSMANAGER_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class Browser;
class MainWindow;

class MyNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MyNetworkAccessManager(QObject * parent = NULL);
    ~MyNetworkAccessManager();
    Browser*        getBrowser();
    MainWindow*     getMainWindow();
    void            setBrowser(Browser* b);
    void            setMainWindow(MainWindow* m);
    QString         createGooglePREFCookie();
    QNetworkReply*  createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0);
public slots:
    void            activateMixedContentBlocking(bool);
    void            URLbarChanged(const QString& url);
    void            zeroTrackerCount();
    void            loadStarted();
signals:
    void            dataReady(const QStringList&, const QColor&);
    void            halfEncryptedURLFound(const QString&);
    void            numberOfTrackers(int);
    void            trackerBlocked(const QStringList&);
private:
    QString         URL;
    MainWindow*     mainWindow;
    Browser*        browser;
    bool            MixedContentBlockerActive;
    bool            pageLoadingStarted;
    quint32         totalTrackers;
    QSet<QString>   blockedURLsPerPage;    // <--- This is per PAGE! NOT all the trackers. It will be zeroes each time urlChanged()
};

#endif // MYNETWORKACCESSMANAGER_H
