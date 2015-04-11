/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "worker.h"

#include <QNetworkProxy>
#include <QNetworkReply>
#include <QDebug>
#include <QColor>

#include <QRegularExpression>
#include <QRegularExpressionMatch>

int Worker::NUMBER_OF_PROXIES = 0;

#include "colors.h"

Worker::Worker(const QList<QString>& p):gi(NULL)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

/*****************************************/
/* GeoIP sanity checking */
/*****************************************/
    if(this->gi == NULL) {
        this->gi = GeoIP_open("./GeoIP.dat",GEOIP_INDEX_CACHE | GEOIP_CHECK_CACHE);
        if(gi == NULL) {
            qDebug() << "Error opening database!!!";
        }

        /* make sure GeoIP deals with invalid query gracefully */
        if (GeoIP_country_code_by_addr(this->gi, NULL) != NULL) {
            qDebug() <<  "Invalid Query test failed, got non NULL, expected NULL";
        }

        if (GeoIP_country_code_by_name(this->gi, NULL) != NULL) {
            qDebug() << "Invalid Query test failed, got non NULL, expected NULL";
        }
    }

    register size_t  n = p.count();
    for(register unsigned int i = 0;i < n;++i) {
        QStringList host = p.at(i).split(QLatin1String(":"),QString::SkipEmptyParts);
        QNetworkAccessManager*  m = new QNetworkAccessManager(this);
        m->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy,host.at(0),host.at(1).toUInt()));
        this->proxies.append(m);
    }
}

QString
Worker::getProxyConnection()const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->proxyConnection;
}

void
Worker::setProxyConnection(const QString& proxyConnection)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->proxyConnection = proxyConnection;
}

QString
Worker::getProxySSL()const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return this->proxySSL;
}

void
Worker::setProxySSL(const QString& proxySSL)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->proxySSL = proxySSL;
}

QString
Worker::getProxyJudge()const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->proxyJudge;
}

void
Worker::setProxyJudge(const QString& proxyJudge)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->proxyJudge = proxyJudge;
}


/* This does all the real work. The basic idea should be that for each
 * proxy listed in this->proxies, three QNetworkRequests are sended simultanely:
 *
 * - one for checking if proxy can be connected
 * - one for checking TLS/SSL status
 * - and one for checking proxy anonymous status (transparent a.k.a. non-anonymous, anonymous or high anonymous)

 * Keep in mind that each of these three individual checks can fail because proxy blocking policies.
 * So they are not dependent of each other.

 * In otherwords, proxy might allow connection to wikipedia.org for example, and it would report
 * "200 OK" in HTTP status, but it could be blocking HTTPS access to the same wikipedia.org

 * Also, even if the proxy reports "200 OK" when connecting, in reality, it might really mean that
 * the proxy is alive but needs username & password to allow further access.
 *
 * TODO: Try to device some means to detect these cases and report it to user.

 * FIXME!!!: Looping throught proxy list and sending three QNetworkRequest for each proxy and
 * waiting responds is fast but hard to synchronize and, it seems, not very robust solution
 * (crashes, no way of setting TTL for requests in Qt, etc....).
 *
 * Also, this particular solution leads to ugly code duplication for all the three tests:
 * Worker::slot_finished(), Worker::slot_encrypted() and Worker::slot_anonymous() to keep
 * things in sync .....
 *
 * There must be a better way!!!
*/

void
Worker::doWork()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Worker::NUMBER_OF_PROXIES = 0;

    connect(this,SIGNAL(allProxiesFinished()),this,SLOT(slot_allProxiesFinished()));

    QEventLoop  loop;
    connect(this,SIGNAL(allProxiesFinished()),&loop,SLOT(quit()));

    register size_t  n = this->proxies.count();
    for(register unsigned int i = 0;i < n;++i) {

        QNetworkRequest req(QUrl(this->proxyConnection));
        req.setPriority(QNetworkRequest::HighPriority);
        QNetworkReply*  r =  this->proxies.at(i)->head(req);

        connect(r,SIGNAL(finished()),this,SLOT(slot_finished()));
        connect(this,SIGNAL(abortConnections()),r,SLOT(abort()));

        QNetworkRequest req2(QUrl(this->proxySSL));
        req2.setPriority(QNetworkRequest::HighPriority);
        QNetworkReply*  ssl = this->proxies.at(i)->head(req2);

        connect(ssl,SIGNAL(finished()),this,SLOT(slot_encrypted()));
        connect(this,SIGNAL(abortConnections()),ssl,SLOT(abort()));

        QNetworkRequest req3(QUrl(this->proxyJudge));
        req3.setPriority(QNetworkRequest::HighPriority);
        QNetworkReply*  anonymous = this->proxies.at(i)->get(req3);

        connect(anonymous,SIGNAL(finished()),this,SLOT(slot_anonymous()));
        connect(this,SIGNAL(abortConnections()),anonymous,SLOT(abort()));

       if(Worker::NUMBER_OF_PROXIES >= this->proxies.count()) {
            break;
        }

    }

    loop.exec();
    emit finished();
}


/* Checks for HTTP status */
void
Worker::slot_finished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

     QNetworkReply *r = qobject_cast<QNetworkReply*>(QObject::sender());
     QSharedPointer<QNetworkReply>  reply(r,&QObject::deleteLater);

     int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
     QString    proxy = reply->manager()->proxy().hostName() + QLatin1String(":") + QString::number(reply->manager()->proxy().port());

     QString    proxyHostname = reply->manager()->proxy().hostName();
     quint32    proxyPort = reply->manager()->proxy().port();

     QStringList    tmp;
     QStringList    data;


     if(reply->error() == QNetworkReply::NoError && httpStatusCode == 200) {
         tmp << "1";
     } else {
         tmp << "0";
     }

     if(httpStatusCode >= 100) {
         tmp <<  (QString::number(httpStatusCode) + QLatin1String(" ") + reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
     } else {
        tmp <<  reply->errorString();
     }

     /* Save the proxy connection status */
     this->connected.insert(proxy,tmp);

    /* Here we already have the results for SSL test & anonymous test.
     * And also the proxy connection test too (line above). So all that is left now is for testing latency (ping)
     * and fetching proxy country from MaxMind GeoIP database and print the results */
     if(this->encrypted.contains(proxy) && this->anonymous.contains(proxy)) {
         // If we have reached here then the QStringList data contains these in the following order:
         // 0 = host
         // 1 = port
         // 2 = http status code
         // 3 = ping
         // 4 = country
         // 5 = ssl
         // 6 = anonymous


         data << proxyHostname << QString::number(proxyPort) << this->connected.value(proxy).at(1);

/* Ping test. This is a blocking operation */
         QString    p;
/* Windows ... */
#ifdef  Q_OS_WIN32
         PICMP_ECHO_REPLY  status = ping(proxyHostname);
         if(status != 0 && status->Status == IP_SUCCESS) {
             p = QString::number(status->RoundTripTime);
         }
#endif
/* ... and Linux */
#ifdef  Q_OS_LINUX
         int  status = ping(proxyHostname.toUtf8().constData());
         if(status > 0) {
             p = QString::number(status);
         }
#endif
         data << p;

/* Country checking from MaxMinds GeoIP database */
         const char* returnedCountry = NULL;
         if(this->gi) {
             returnedCountry =  GeoIP_country_name_by_addr(this->gi,proxyHostname.toLatin1().constData());
         }
         data << returnedCountry;
         data << this->encrypted.value(proxy).at(1) << this->anonymous.value(proxy).at(1);

/* "Mark" the read data either with green (if *all* the three checks passed, otherwise with red */
         QColor  color;

         if(this->connected.value(proxy).at(1).startsWith(QLatin1String("200 OK")) && this->encrypted.value(proxy).at(1) == QLatin1String("Yes") && (this->anonymous.value(proxy).at(1) == QLatin1String("High-anonymous") || this->anonymous.value(proxy).at(1) == QLatin1String("Anonymous")) ) {
             color = MediumSpringGreen;
         } else {
             color = ChilliPepper;
         }

         /* Data is now ready to send to main GUI thread! */
         emit dataReady(data,color);

         /* Update number of proxies that have been handled so far */
         Worker::NUMBER_OF_PROXIES++;

         qDebug() << "# OF PROXIES  " << Worker::NUMBER_OF_PROXIES;

         /* Has the number of proxies checked reached the total amount of proxies in the list?
          * If so, then send signal and inform main GUI thread*/
         if(Worker::NUMBER_OF_PROXIES >= this->proxies.count()) {
             emit allProxiesFinished();
         }
     }
}

/* Checks for TLS/SSL status of proxy */
void
Worker::slot_encrypted()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QNetworkReply *r = qobject_cast<QNetworkReply*>(QObject::sender());
    QSharedPointer<QNetworkReply>  reply(r,&QObject::deleteLater);

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString    proxy = reply->manager()->proxy().hostName() + QLatin1String(":") + QString::number(reply->manager()->proxy().port());

    QString    proxyHostname = reply->manager()->proxy().hostName();
    quint32    proxyPort = reply->manager()->proxy().port();

    QStringList tmp;
    QStringList data;

    if(reply->error() == QNetworkReply::NoError && httpStatusCode == 200) {
        tmp << "1";
    } else {
        tmp << "0";
    }

    if(httpStatusCode >= 100) {
        if(httpStatusCode == 200) {
            tmp << "Yes";
        } else {
            tmp <<  (QString::number(httpStatusCode) + QLatin1String(" ") + reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        }
    } else {
               tmp <<  reply->errorString();
    }

    /* Save the proxy TLS/SSL status */
    this->encrypted.insert(proxy,tmp);

    /* Here we already have the results for connection (HTTP status) test & anonymous test.
     * And also the proxy SSL test too (line above). So all that is left now is for testing latency (ping)
     * and fetching proxy country from MaxMind GeoIP database and print the results */
    if(this->connected.contains(proxy) && this->anonymous.contains(proxy)) {
        // If we have reached here then the QStringList data contains these in the following order:
        // 0 = host
        // 1 = port
        // 2 = http status code
        // 3 = ping
        // 4 = country
        // 5 = ssl
        // 6 = anonymous
        data << proxyHostname << QString::number(proxyPort) << this->connected.value(proxy).at(1);

/* Ping test. This is a blocking operation */
        QString    p;
/* Windows ... */
#ifdef  Q_OS_WIN32
        PICMP_ECHO_REPLY  status = ping(proxyHostname);
        if(status != 0 && status->Status == IP_SUCCESS) {
            p = QString::number(status->RoundTripTime);
        }
#endif
/* ... and Linux */
#ifdef  Q_OS_LINUX
        int  status = ping(proxyHostname.toUtf8().constData());
        if(status > 0) {
            p = QString::number(status);
        }
#endif
        data << p;

/* Country checking from MaxMinds GeoIP database */
        const char* returnedCountry = NULL;
        if(this->gi) {
            returnedCountry =  GeoIP_country_name_by_addr(this->gi,proxyHostname.toLatin1().constData());
        }
        data << returnedCountry;
        data << this->encrypted.value(proxy).at(1) << this->anonymous.value(proxy).at(1);


/* "Mark" the read data either with green (if *all* the three checks passed, otherwise with red */
        QColor  color;
        if(this->connected.value(proxy).at(1).startsWith(QLatin1String("200 OK")) && this->encrypted.value(proxy).at(1) == QLatin1String("Yes") && (this->anonymous.value(proxy).at(1) == QLatin1String("High-anonymous") || this->anonymous.value(proxy).at(1) == QLatin1String("Anonymous")) ) {
            color = MediumSpringGreen;
        } else {
            color = ChilliPepper;
        }

        /* Data is now ready to send to main GUI thread! */
        emit dataReady(data,color);

        /* Update number of proxies that have been handled so far */
        Worker::NUMBER_OF_PROXIES++;

        qDebug() << "# OF PROXIES  " << Worker::NUMBER_OF_PROXIES;

        /* Has the number of proxies checked reached the total amount of proxies in the list?
         * If so, then send signal and inform main GUI thread*/
        if(Worker::NUMBER_OF_PROXIES >= this->proxies.count()) {
            emit allProxiesFinished();
        }

    }
}

void
Worker::slot_anonymous()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QNetworkReply *r = qobject_cast<QNetworkReply*>(QObject::sender());
    QSharedPointer<QNetworkReply>  reply(r,&QObject::deleteLater);

    int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QString    proxy = reply->manager()->proxy().hostName() + QLatin1String(":") + QString::number(reply->manager()->proxy().port());

    QString    proxyHostname = reply->manager()->proxy().hostName();
    quint32    proxyPort = reply->manager()->proxy().port();


    QStringList tmp;
    QStringList data;

    QString  rawData;

    if(reply->error() == QNetworkReply::NoError && httpStatusCode == 200) {
        tmp << "1";
        rawData = reply->readAll(); /* Get the raw HTML data of proxy response so that we can try to parse it and
                                      check anonymous status below */
        qDebug() << rawData;
    } else {
        tmp << "0";
    }

    if(httpStatusCode >= 100) {
/* =====================================================*/
/* Proxy anononymous checker.
 * The logic is pretty simple:
 * - for each proxy in the list try to connect to some proxy judge site (user definable)
 *   and check for it's REMOTE_ADDR field. If it matches the proxy address then the
 *   proxy is at least anonymous. If the proxy respond also does not contain any of the
 *   dangerous_proxy_headers (see below) then it is also high-anonymous proxy.
/* =====================================================*/

        /* By default, all proxies are marked as non-anonymous and depending of the results of checks below,
         * they might (or might not) gain Anonymous or High-anonymous status */
        QString anon = "No";


        QRegularExpression re(QLatin1String("REMOTE_ADDR = (.*)"));
        QRegularExpressionMatch match = re.match(rawData);
        if (match.hasMatch()) {
            qDebug() << "WHOLE MATCH " << match.captured(0);
            qDebug() << "EXACT MATCH " << match.captured(1);
            if(proxyHostname == match.captured(1)) {
                anon = "High-anonymous";
            }
        }

        /* TODO: Add new field to Proxy GUI where user can add/change/update this for future proof & increased flexibility */
        QRegularExpression dangerous_proxy_headers(QLatin1String("X-Forwarded-For|HTTP_FORWARDED|HTTP_X_FORWARDED_FOR|HTTP_X_PROXY_ID|HTTP_VIA|CLIENT_IP|HTTP_FROM"));

        QRegularExpressionMatch match2 = dangerous_proxy_headers.match(rawData);
        if(match2.hasMatch()) {
            anon = "Anonymous";
        }

        tmp << anon;
        /* =====================================================*/
    } else {
        tmp <<  reply->errorString();
    }


    /* Save the proxy anonymous status */
    this->anonymous.insert(proxy,tmp);


    /* Here we already have the results for connection (HTTP status) test & TLS/SSL test.
     * And also the proxy anonymous test too (line above). So all that is left now is for testing latency (ping)
     * and fetching proxy country from MaxMind GeoIP database and print the results */
    if(this->connected.contains(proxy) && this->encrypted.contains(proxy)) {
        // If we have reached here then the QStringList data contains these in the following order:
        // 0 = host
        // 1 = port
        // 2 = http status code
        // 3 = ping
        // 4 = country
        // 5 = ssl
        // 6 = anonymous

        data << proxyHostname << QString::number(proxyPort);
        data << this->connected.value(proxy).at(1);

/* Ping test. This is a blocking operation */
        QString    p;
/* Windows ... */
#ifdef  Q_OS_WIN32
        PICMP_ECHO_REPLY  status = ping(proxyHostname);
        if(status != 0 && status->Status == IP_SUCCESS) {
            p = QString::number(status->RoundTripTime);
        }
#endif
/* ... and Linux */
#ifdef  Q_OS_LINUX
        int  status = ping(proxyHostname.toUtf8().constData());
        if(status > 0) {
            p = QString::number(status);
        }
#endif
        data << p;

/* Country checking from MaxMinds GeoIP database */
        const char* returnedCountry = NULL;
        if(this->gi) {
            returnedCountry =  GeoIP_country_name_by_addr(this->gi,proxyHostname.toLatin1().constData());
        }
        data << returnedCountry;
        data << this->encrypted.value(proxy).at(1);

        /* Checking proxy anonymous status. Yes, this is exact duplicate code of above because
           trying to sync QNetwork replies and keep things in order */
        if(this->anonymous.value(proxy).at(0) == QLatin1String("1")) {

            QString anon = "No";

            QRegularExpression re(QLatin1String("REMOTE_ADDR = (.*)"));
            QRegularExpressionMatch match = re.match(rawData);
            if (match.hasMatch()) {
                qDebug() << "WHOLE MATCH " << match.captured(0);
                qDebug() << "EXACT MATCH " << match.captured(1);
                if(proxyHostname == match.captured(1)) {
                    anon = "High-anonymous";
                }
            }

            QRegularExpression dangerous_proxy_headers(QLatin1String("X-Forwarded-For|HTTP_FORWARDED|HTTP_X_FORWARDED_FOR|HTTP_X_PROXY_ID|HTTP_VIA|CLIENT_IP|HTTP_FROM"));
            QRegularExpressionMatch match2 = dangerous_proxy_headers.match(rawData);
            if(match2.hasMatch()) {
                anon = "Anonymous";
            }

            data << anon;
        } else {
            data <<  this->anonymous.value(proxy).at(1);
        }


/* "Mark" the read data either with green (if *all* the three checks passed, otherwise with red */
        QColor  color;
        if(this->connected.value(proxy).at(1).startsWith(QLatin1String("200 OK")) && this->encrypted.value(proxy).at(1) == QLatin1String("Yes") && (this->anonymous.value(proxy).at(1) == QLatin1String("High-anonymous") || this->anonymous.value(proxy).at(1) == QLatin1String("Anonymous")) ) {
            color = MediumSpringGreen;
        } else {
            color = ChilliPepper;
        }

        /* Data is now ready to send to main GUI thread! */
        emit dataReady(data,color);

        /* Update number of proxies that have been handled so far */
        Worker::NUMBER_OF_PROXIES++;

        qDebug() << "# OF PROXIES  " << Worker::NUMBER_OF_PROXIES;
        /* Has the number of proxies checked reached the total amount of proxies in the list?
         * If so, then send signal and inform main GUI thread*/
        if(Worker::NUMBER_OF_PROXIES >= this->proxies.count()) {
            emit allProxiesFinished();
        }

    } else {

    }
}



void
Worker::slot_allProxiesFinished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "ALL " << Worker::NUMBER_OF_PROXIES << " PROXIES FINISHED";
}



void
Worker::slot_abortConnections()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    // Stop all pending requests ... or at least try to stop them ...
    Worker::NUMBER_OF_PROXIES = this->proxies.count();
    emit abortConnections();
    emit allProxiesFinished();
}
