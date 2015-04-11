/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MYCOOKIEJAR_H
#define MYCOOKIEJAR_H

#include <QNetworkCookieJar>
#include <QNetworkCookie>
#include <QStringList>
#include <QHash>

class MainWindow;

class MyCookieJar : public QNetworkCookieJar
{
    Q_OBJECT;
    QHash<uint,QNetworkCookie>    currentCookies;
    QHash<uint,QNetworkCookie>    totalCookies;
public:
    explicit                MyCookieJar(QObject* parent = NULL);
    virtual                 ~MyCookieJar();
    void                    setMainWindow(MainWindow* m);
    QList<QNetworkCookie> 	allCookies() const;
    void                    emptyCurrentCookies();
    bool                    insertCookie(const QNetworkCookie & cookie);
    void                    setAllCookies(const QList<QNetworkCookie> & cookieList);
    QList<QNetworkCookie>   cookiesForUrl(const QUrl & url) const;
signals:
    void                    cookieUpdated(QStringList);
    void                    updateAllCookiesList(QList<QNetworkCookie>);
public slots:
    size_t                  deleteAllCookies();
private slots:
private:
    MainWindow*             mainWindow;
};

#endif // MYCOOKIEJAR_H
