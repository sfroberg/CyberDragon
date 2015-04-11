/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "cookie.h"
#include "ui_cookie.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mycookiejar.h"
#include "browser.h"

#include <QDebug>
#include <QDateTime>

inline uint qHash(const QNetworkCookie& key, uint seed = 0)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return qHash(key.domain() +key.path() + key.name() + key.value(),seed);
}


void    MyCookieJar::emptyCurrentCookies()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->currentCookies.clear();
}

MyCookieJar::MyCookieJar(QObject* parent):
    QNetworkCookieJar(parent),mainWindow(NULL)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}

MyCookieJar::~MyCookieJar()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}

void
MyCookieJar::setMainWindow(MainWindow* m)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->mainWindow = m;
}


/* Ingoing cookie */
bool
MyCookieJar::insertCookie(const QNetworkCookie & cookie)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(!currentCookies.contains(qHash(cookie))) {
        currentCookies.insert(qHash(cookie),cookie);
    }

    if(!totalCookies.contains(qHash(cookie))) {
        totalCookies.insert(qHash(cookie),cookie);
    }

    QStringList	data;
    data << QDateTime::currentDateTime().toString()  << "Allowed" << cookie.domain();
    data << cookie.path() << QString(cookie.name()) + "=" + QString(cookie.value()) << cookie.expirationDate().toString() << (cookie.isSecure() ? QLatin1String("Yes") : QLatin1String("No")) << (cookie.isHttpOnly() ? QLatin1String("Yes") : QLatin1String("No"));



/**************************/
/* Cookie checking begins */
/**************************/

/* Very first, check if the url is found from custom cookie rules that will override everything else */
/* If matching domain is found (NOTE: it's cookie DOMAIN vs. custom cookie URL regexp match!) then
 * check if Allow is checked. */
/* Then, if cookie is checked as Allowed then do regexp check of Name-Value */
/* Finally, if Name=Value is also ok then do return insertCookie() */

    Q_ASSERT(this->mainWindow->cookieTab != NULL);

    register size_t n = this->mainWindow->cookieTab->ui->CustomCookieRules_treeWidget->topLevelItemCount();
    for(register unsigned int i = 0;i < n;++i) {

        QTreeWidgetItem*    tmp = this->mainWindow->cookieTab->ui->CustomCookieRules_treeWidget->topLevelItem(i);


        QRegularExpression          reg(tmp->text(1));
        QRegularExpressionMatch     match = reg.match(cookie.domain());
        /* First check domain */
        if(match.hasMatch()) {
            /* Then check path */
            QRegularExpression  path(tmp->text(2));
            match = path.match(cookie.path());
            if(match.hasMatch()) {
                /* Finally check name-value pair */
                QRegularExpression  namevalue(tmp->text(3));
                match = namevalue.match(cookie.name() + QLatin1String("=") + cookie.value());
                if(match.hasMatch()) {
                    /* And lastly, either allow cookie insertCookie() or deny it return false */
                    if(tmp->text(0) == QLatin1String("Allowed")) {
                        emit cookieUpdated(data);
                        emit updateAllCookiesList(this->allCookies());

                        return  QNetworkCookieJar::insertCookie(cookie);
                    } else {
                        data[1] = "Blocked";
                        /* Add 8th member, the reason why cookie was blocked */
                        data << "custom cookie rule";
                        emit cookieUpdated(data);
                        return false;
                    }

                }
            }
        }
    }

/**************************/
/* Global Cookie checking begins */
/**************************/
    /* 3rd party check */
    if(!this->mainWindow->cookieTab->ui->Allow3rdPartyCookies_checkBox->isChecked()) {

        if(!this->mainWindow->currentTab()->getUrl().host().isEmpty()) {
            /* check cookie domain vs. url */
            QRegularExpression          reg(cookie.domain());
            QRegularExpressionMatch     match = reg.match(this->mainWindow->currentTab()->getUrl().host());
            if(!match.hasMatch()) {
                qDebug() << " COOKIE " << cookie << " BLOCKED!!!!!!";

                data[1] = "Blocked";

                /* Add 8th member, the reason why cookie was blocked */
                data << "3rd party cookie";
                emit cookieUpdated(data);
                return  false;
            }
        }
    }

    /* If HttpOnly cookie has been checked and cookie has not that attribute set then block the cookie */
    if(this->mainWindow->cookieTab->ui->HttpOnlyCookie_checkBox->isChecked() && !cookie.isHttpOnly()) {
        qDebug() << " COOKIE " << cookie << " BLOCKED!!!!!!";
        data[1] = "Blocked";
        /* Add 8th member, the reason why cookie was blocked */
        data << "HttpOnly attribute not set";
        emit cookieUpdated(data);
        return  false;
    }

    /* If Session only cookie has been checked and cookie is not session cookie (it has set date) then block the cookie */
    if(this->mainWindow->cookieTab->ui->SessionCookie_checkBox->isChecked() && !cookie.isSessionCookie()) {
        qDebug() << " COOKIE " << cookie << " BLOCKED!!!!!!";
        data[1] = "Blocked";
        /* Add 8th member, the reason why cookie was blocked */
        data << "non-session cookie";
        emit cookieUpdated(data);
        return  false;
    }

    /* If Secure only cookie has been checked and cookie has not that attribute set then block the cookie */
    if(this->mainWindow->cookieTab->ui->SecureCookie_checkBox->isChecked() && !cookie.isSecure()) {
        qDebug() << " COOKIE " << cookie << " BLOCKED!!!!!!";
        data[1] = "Blocked";
        /* Add 8th member, the reason why cookie was blocked */
        data << "Secure attribute not set";
        emit cookieUpdated(data);
        return  false;
    }

    qDebug() << " COOKIE " << cookie << " ALLOWED!";

    /* If we get this far then do the default cookie thingy ... */
    emit cookieUpdated(data);
    emit updateAllCookiesList(this->allCookies());
    return  QNetworkCookieJar::insertCookie(cookie);
}

/* Outgoing cookie.
 *
 * FIXME: We should really do similar cookie filtering here too, like with
 * insertCookie((const QNetworkCookie&) above, because if user accidentally (or intentionally) allows
 * some cookie(s) and then later blocks them again, it won't do any good.

 * Reason being because the cookies are already in memory after they have been allowed (incoming).
 * The only fix to that right now is to empty all cookies, close the open tab (cookies are tab specific in CyberDragon)
 * or close and start CyberDragon. Those operations will always destroy all cookies in memory.
 */
QList<QNetworkCookie> MyCookieJar::cookiesForUrl(const QUrl & url) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
   QList<QNetworkCookie>   cookies =  QNetworkCookieJar::cookiesForUrl(url);
   return cookies;
}

QList<QNetworkCookie>
MyCookieJar::allCookies() const {
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  QNetworkCookieJar::allCookies();
}

void
MyCookieJar::setAllCookies(const QList<QNetworkCookie> & cookieList)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QNetworkCookieJar::setAllCookies(cookieList);
}

size_t
MyCookieJar::deleteAllCookies()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
     QList<QNetworkCookie>   cookies = this->allCookies();
     size_t  n = 0;
     foreach (QNetworkCookie cookie, cookies) {
         QNetworkCookieJar::deleteCookie(cookie);
         ++n;
     }
     return n;
}
