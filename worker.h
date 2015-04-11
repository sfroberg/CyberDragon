/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef WORKER_H
#define WORKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QEventLoop>
#include <QTimer>

#ifdef  Q_OS_WIN32
#include <winsock2.h>
#include <windows.h>
#include <Iphlpapi.h>
#include <icmpapi.h>
#endif

#include <zlib.h>
#include <GeoIP.h>

#include "ping.h"


class Worker : public QObject
{
    Q_OBJECT
public:
    static  int NUMBER_OF_PROXIES;
    Worker(const QList<QString>& proxies);
    QString getProxyConnection()const;
    void    setProxyConnection(const QString& proxyConnection);
    QString getProxySSL()const;
    void    setProxySSL(const QString& proxySSL);
    QString getProxyJudge()const;
    void    setProxyJudge(const QString& proxyJudge);
signals:
    void    abortConnections();
    void    allProxiesFinished();
    void    dataReady(const QStringList&,const QColor&);
    void    finished();
public slots:
    void    doWork();
    void    slot_finished();
    void    slot_encrypted();
    void    slot_anonymous();
    void    slot_allProxiesFinished();
    void    slot_abortConnections();
private:
    QList<QNetworkAccessManager*>   proxies;
    QString                         proxyConnection;
    QString                         proxySSL;
    QString                         proxyJudge;
    QHash<QString,QStringList>      connected;
    QHash<QString,QStringList>      encrypted;
    QHash<QString,QStringList>      anonymous;
    GeoIP*                          gi;
};

#endif // WORKER_H
