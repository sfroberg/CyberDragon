/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>
#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QNetworkAccessManager>
#include <QNetworkDiskCache>

#include "mynetworkaccessmanager.h"
#include "mywebpage.h"
#include "mycookiejar.h"

#include "serverinfo.h"

namespace Ui {
class Browser;
}

class Browser : public QWidget
{
    Q_OBJECT
public:
    explicit Browser(QWidget *parent = 0, const QString& url = QLatin1String("about:blank"));
    ~Browser();
    MyNetworkAccessManager* getNetworkAccessManager() const;
    QWebFrame*              getWebFrame() const;
    QWebPage*               getWebPage() const;
    QWebView*               getWebView() const;
    QString                 getTitle() const;
    QUrl                    getUrl()const;
    MyCookieJar*            getCookieJar()const;
    MainWindow*             getMainWindow();
    void                    setMainWindow(MainWindow* m);
    QString                 getCache() const;
    void                    setCache();
    void                    clearCache();
    ServerInfo*             getServerInfo();
public slots:
    void                    loadFinished(bool ok);
    void                    slot_loadStarted();
    void                    returnPressed();
    void                    slot_frame_loadStarted();
    void                    slot_frame_loadFinished(bool ok);
    void                    slot_urlChanged(const QUrl& url);
    void                    checkTrackers(QNetworkReply* reply);
    void                    updateTrackersNumber(int n);
    void                    urlFocus();
    void                    handlelinkClicked(const QUrl & url);
    void                    slot_forward();
    void                    slot_backward();
    /**/
    void                    finished(QNetworkReply * reply);
    void                    sslErrors(QNetworkReply * r, const QList<QSslError> & errors);
    /* */
    void                    checkEncrypted(QNetworkReply* r);
    /* */
    void                    authenticate(QNetworkReply * reply, QAuthenticator * authenticator);
signals:
    /* Delegate all these signals to upwards ... (aka all the various child elements signals
     * that this Browser widget owns) */
    void                    linkHovered(const QString& link,const QString& title, const QString& textContent);
    void                    loadStarted();
    void                    titleChanged(const QString& title);
    void                    urlChanged(const QUrl& url);
    void                    unsupportedContent(QNetworkReply * reply);
    /**********************************/
    void                    activateMixedContentBlocking(bool);
    void                    clearTrackerList();
    void                    clearMixedContentBlockingList();
    void                    clearCookieList();
    /**********************************/
    void                    cipherDetailsChanged(const QString&);
    /* Log stuff */
    void                    dataReady(const QStringList& data,const QColor& color);
private slots:
    void                    on_Home_pushButton_clicked();
private:
    friend class            general;
    friend class            MyNetworkAccessManager;
    MainWindow*             mainWindow;
    MyCookieJar*            cookieJar;
    MyNetworkAccessManager* manager;
    MyWebPage*              page;
    QUrl                    mainURL;
    ServerInfo*             serverInfo;
    QSet<QWebHistoryItem>   historyList;
    Ui::Browser             *ui;
};

#endif // BROWSER_H
