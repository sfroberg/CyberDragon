/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */


#include "ftpreply.h"
#include "sandboxreply.h"
#include "mynetworkaccessmanager.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "general.h"
#include "ui_general.h"

#include "browser.h"
#include "ui_browser.h"

#include "trackerblocker.h"
#include "ui_trackerblocker.h"

#include "encryption.h"
#include "ui_encryption.h"

#include "proxy.h"
#include "ui_proxy.h"

#include "cookie.h"
#include "ui_cookie.h"

#include "cache.h"
#include "ui_cache.h"

#include "notification.h"
#include "ui_notification.h"

#include "log.h"
#include "ui_log.h"

#include <QDebug>
#include <QWebView>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

#include "colors.h"

QString
MyNetworkAccessManager::createGooglePREFCookie()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    Q_ASSERT(this->mainWindow->cookieTab != NULL);
    QString   cookie2 = "PREF=ID=";

    QString ID = QCryptographicHash::hash(QString::number(QTime::currentTime().msec()).toLatin1(),QCryptographicHash::Md5).toHex();
    ID.truncate(16);
    cookie2 = cookie2 + ID + ":FF=4:LD=" + this->mainWindow->cookieTab->ui->SpoofGooglePREFCookie_comboBox->currentText() + ":NR=50:TM=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000) + ":LM=" + QString::number(QDateTime::currentDateTime().toMSecsSinceEpoch()/1000) + ":GBV=1:SG=1:S=";
    QString   S = cookie2.toLatin1().toBase64();
    S.truncate(16);
    cookie2 = cookie2 + S;
    return  cookie2;
}

MyNetworkAccessManager::MyNetworkAccessManager(QObject * parent):
    QNetworkAccessManager(parent),mainWindow(NULL),browser(NULL),MixedContentBlockerActive(false),totalTrackers(0)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}

MyNetworkAccessManager::~MyNetworkAccessManager()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}

Browser*
MyNetworkAccessManager::getBrowser()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->browser;
}

MainWindow*
MyNetworkAccessManager::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->mainWindow;
}

void
MyNetworkAccessManager::setBrowser(Browser* b)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->browser = b;
}

void
MyNetworkAccessManager::setMainWindow(MainWindow* m)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->mainWindow = m;
}

void
MyNetworkAccessManager::zeroTrackerCount()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->totalTrackers = 0;

    /* Also empty total trackers per Page */
    this->blockedURLsPerPage.clear();
}

/* Most time critical function in whole CyberDragon. Try to keep as optimized as possible */
QNetworkReply*
MyNetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << req.url();

    QNetworkRequest r(req);
    QString         scheme = r.url().scheme();

    /* Sanity checks. Compiler will optimize these away in release code.
     * These should never be NULL */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->browser != NULL);
    Q_ASSERT(this->mainWindow->logTab != NULL);
    Q_ASSERT(this->mainWindow->trackerblockerTab != NULL);
    Q_ASSERT(this->mainWindow->cacheTab != NULL);
    Q_ASSERT(this->mainWindow->proxyTab != NULL);
    Q_ASSERT(this->mainWindow->encryptionTab != NULL);
    Q_ASSERT(this->mainWindow->notificationTab != NULL);
    Q_ASSERT(this->mainWindow->cookieTab != NULL);
    Q_ASSERT(this->mainWindow->generalTab != NULL);



/****************************************************************
 * Disable Google Search Autocomplete
 * **************************************************************/

    QString gooleSearchAutocomplete(r.url().host() + r.url().path());
    if(this->mainWindow->generalTab->ui->DisableGoogleSearchAutocomplete_checkBox->isChecked() && gooleSearchAutocomplete.startsWith(QLatin1Literal("clients1.google.com/complete"))) {
        r.setUrl(QUrl(QLatin1String("")));
        return QNetworkAccessManager::createRequest( op, r, outgoingData );
    }


    /* FTP handling */
    if(scheme == QLatin1String("ftp")) {
        if (op == QNetworkAccessManager::GetOperation)
            return new FtpReply(r.url());
        else
            return QNetworkAccessManager::createRequest(op, r, outgoingData);
    }


    if(this->mainWindow->logTab->ui->enableLog_checkBox->isChecked()) {
        QStringList data;
        data << QDateTime::currentDateTime().toString() << "Started loading " << "" << r.url().toDisplayString();
        emit dataReady(data,QColor(0,0,0));
    }

/***********************************************************
 * Tracker Blocker
 * *********************************************************/
    /* We only handle tracker blocking for HTTP and HTTPS protos. Skip for all the rest */
    if(scheme != QLatin1String("http") && scheme != QLatin1String("https"))    {
        return QNetworkAccessManager::createRequest( op, r, outgoingData );
    }

    if(this->mainWindow->trackerblockerTab->ui->BlockTrackers_checkBox->isChecked()) {

        /* Before we go any further, first check if the tracker has already been blocked before.
         * This is because some web pages have an annoying habit of keep trying to push the same
         * tracker(s) over and over .... */
        if(this->blockedURLsPerPage.contains(r.url().toDisplayString())) {
            r.setUrl(QUrl(QLatin1String("")));
            return QNetworkAccessManager::createRequest( op, r, outgoingData );
        }

        /* Ok, not previously blocked. Let's continue ... */
        register size_t  n = this->mainWindow->trackerblockerTab->ui->TrackerBlockerRules_treeWidget->topLevelItemCount();

        static QRegularExpressionMatch match;
        static  QRegularExpression  regExp;

        /* Loop the rule list, set regular expression pattern and test for match */
        for(register unsigned int i = 0;i < n;++i) {

            QTreeWidgetItem*    item = this->mainWindow->trackerblockerTab->ui->TrackerBlockerRules_treeWidget->topLevelItem(i);

            if(item->checkState(0) == Qt::Checked) {

            regExp.setPattern(item->text(0));

            if(!regExp.isValid()) {
                qDebug() << "ERROR!!! INVALID REGEXP " << regExp.pattern();
            }

            /* Some minor micro optimization. Have not benchmarked this.
             * Idea is that if the first character of the regexp rule is '.' then it's a general rule and only URL path part is used.
             * If it starts with '^' then it's a domain/subdomain rule and only URL host part is used.
             * And for all the rest it's the full URL minus scheme (tracker blocker does not care about scheme at all).
             * Even tought the speed gain might be miniscule this should still decrease the chance of false matches
             * when doing regexp */
            if(regExp.pattern().at(0) == '.')                /* Generic match against path part of URL only */
                    match = regExp.match(r.url().path());
                else if(regExp.pattern().at(0) == '^')           /* Domain/subdomain match against host part of URL only */
                    match = regExp.match(r.url().host());
                else                                    /* Specific match against whole host + path of URL */
                    match = regExp.match(r.url().host() + r.url().path());

                if(match.hasMatch()) {
                        this->totalTrackers++;
                        emit numberOfTrackers(this->totalTrackers);
                        QStringList text;
                        text << r.url().toString() << regExp.pattern();
                        emit trackerBlocked(text);
                        this->blockedURLsPerPage.insert(r.url().toDisplayString());

                        if(this->mainWindow->logTab->ui->enableLog_checkBox->isChecked()) {
                            QStringList data;
                            data << QDateTime::currentDateTime().toString() << "Blocked by Tracker Blocker" << "" << r.url().toDisplayString();
                            emit dataReady(data,ChilliPepper);
                        }

                        r.setUrl(QUrl(QLatin1String("")));
                        return QNetworkAccessManager::createRequest( op, r, outgoingData );
                }
            }
                //++i;
        }
    }

/***********************************************************
 * MCB (Mixed Content Blocker)
 * *********************************************************/
/* TODO: Add WebSocket Mixed Content blocking */

    /* Test: https://www.ssllabs.com/ssltest/viewMyClient.html */
    if(this->mainWindow->encryptionTab->ui->MCB_checkBox->isChecked() && this->MixedContentBlockerActive) {
        QUrl    url(this->URL);

        if(url.scheme() == QLatin1String("https"))
        if(scheme == QLatin1String("http") || scheme == QLatin1String("ws")) {

            /* First check if there is MCB exception in list */
            QRegularExpressionMatch match2;
            QRegularExpressionMatch match3; // for special cases, like ixquick-proxy.com which often
                                            // proxies https ---> http

            QRegularExpression   reg2;


            register size_t  n2 = this->mainWindow->encryptionTab->ui->DisableMCB_treeWidget->topLevelItemCount();
            for(register unsigned int i = 0;i < n2;++i) {
                if(this->mainWindow->encryptionTab->ui->DisableMCB_treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked) {
                    reg2.setPattern(this->mainWindow->encryptionTab->ui->DisableMCB_treeWidget->topLevelItem(i)->text(0));
                    match2 = reg2.match(r.url().host() + r.url().path());
                    if(match2.hasMatch()) {
                        return QNetworkAccessManager::createRequest( op, r, outgoingData );
                    }
                    match3 = reg2.match(url.host() + url.path());
                    if(match3.hasMatch()) {
                        return QNetworkAccessManager::createRequest( op, r, outgoingData );
                    }

                }
            }
            /* Ok, no MCB exception were found. Notify user and block connection */
                emit halfEncryptedURLFound(r.url().toString());                               

                if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyMixedContent_checkBox->isChecked()) {
                    Notify* notify = new Notify(QLatin1String("Mixed HTTPS Content Found!"),"The following URL uses unencrypted http:// schema: " + r.url().toString());
                    notify->show();
                }

                if(this->mainWindow->logTab->ui->enableLog_checkBox->isChecked()) {
                    QStringList data;
                    data << QDateTime::currentDateTime().toString() << "Blocked by MCB" << "" << r.url().toDisplayString();
                    emit dataReady(data,ChilliPepper);
                }
                r.setUrl(QUrl(QLatin1String("")));
                return  QNetworkAccessManager::createRequest(op,r,outgoingData);               
        }
    }

    /* Google PREF cookie spoofing */
    if(this->mainWindow->cookieTab->ui->SpoofGooglePREFCookie_checkBox->isChecked()) {
        QRegularExpression  googleFilter(QLatin1String("^(.+\\.)*google\\.{2,3}"));
        QRegularExpressionMatch googleMatch = googleFilter.match(r.url().host());
        if (googleMatch.hasMatch()) {
            if(this->mainWindow->cookieTab->ui->ManuallySpoofGooglePREFCookie_radioButton->isChecked()) {
                r.setRawHeader("Cookie",this->mainWindow->cookieTab->ui->SpoofGooglePREFCookie_lineEdit->text().toLatin1());
            }

            if(this->mainWindow->cookieTab->ui->RandomSpoofGooglePREFCookie_radioButton->isChecked()) {
                QString tmp = this->createGooglePREFCookie();
                this->mainWindow->cookieTab->ui->SpoofGooglePREFCookie_lineEdit->setText(tmp);
                this->mainWindow->cookieTab->ui->SpoofGooglePREFCookie_lineEdit->setCursorPosition(0);
                r.setRawHeader("Cookie",tmp.toLatin1());
            }
        }
    }
                /* Set HTTP Referer to empty */
                /* Test: http://darklaunch.com/tools/test-referer */
                /* OBS! Undocumented from Qt docs. Setting value to NULL will remove header */

                switch(this->mainWindow->generalTab->httpReferer->checkedId()) {
                    case    HTTPReferer::CustomHTTPReferer:
                    {
                        r.setRawHeader("Referer",this->mainWindow->generalTab->ui->CustomHTTPReferer_lineEdit->text().toUtf8());
                    }
                    break;
                    case    HTTPReferer::SameAsCurrentUrl:
                        r.setRawHeader("Referer",r.url().toDisplayString().toUtf8());
                    break;
                    case    HTTPReferer::Remove:
                        r.setRawHeader("Referer",NULL);
                    break;
                    case    HTTPReferer::DoNotTouch:
                    default:
                        // Do nothing
                    break;
                }



                if(this->mainWindow->generalTab->ui->PreventETags_checkBox->isChecked()) {
                    r.setRawHeader("If-None-Match",NULL);
                }

                /* We always send Do-Not-Track header. Even tought advertisers probably would care less about it ... */
                r.setRawHeader("DNT","1");


                /* HTTP Pipelining */
                if(this->mainWindow->generalTab->ui->HttpPipeline_checkBox->isChecked()) {
                    if(this->mainWindow->proxyTab->ui->Proxy_lineEdit->text().isEmpty() && op != QNetworkAccessManager::PostOperation) {
                        r.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute,true);
                    }
                }

    r.setPriority(QNetworkRequest::HighPriority);

    if(this->mainWindow->cacheTab->ui->noCache_radioButton->isChecked()) {
        r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysNetwork);
    }

    if(this->mainWindow->cacheTab->ui->enableCache_radioButton->isChecked()) {
        r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    }

    if(this->mainWindow->cacheTab->ui->allwaysCache_radioButton->isChecked()) {
        r.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
    }


/* Iframe sandboxing test. VERY EXPERIMENTAL!!!
 *
 * Basic idea here is that if the very first page that browser loads from some site contains
 * iframes (either visible or invisible), then this will try to prevent any hostile content
 * like scripts, plugins etc. from running. And it does it dynamically, just after rendering
 * the very first page that contains <iframe> tags but before it starts loading content into
 * those iframes.
 *
 * Will most likely break pages and their layout or give totally blank pages.
 * However, if you want to test it, first go to http://www.binarytouch.com/iframe_sandbox_test.html
 * Then uncomment the code below, rebuild and visit that page again.
 * If nothing shows up this time then it is working. You can further confirm this by right clicking
 * and choosing "Inspect" and you will see that <iframe> tag has now "sandbox" attribute added to it.

 * To improve this see the ugly messy guts of SandboxReply class from sandboxreply.cpp
 * and especially the slot_Finished() function.

 * Suggestion: When (and if) you get this working, add new tab (like "Iframe protection" or "Iframe sandboxing")
 * add checkbox for enable/disable it and also checkboxes for more finer grainer blocking,
 * like blocking only scripts from iframes, blocking only plugins from iframes, etc......
 *
 * For more info about Iframe sandboxing and why everybody should use it (sadly, most web developers don't use
 * it, that's why this dynamic sandboxing feature), please see:
 * http://www.html5rocks.com/en/tutorials/security/sandboxed-iframes/
*/
/*
    if(this->pageLoadingStarted) {
        QNetworkReply* r1 = QNetworkAccessManager::createRequest(op, r, outgoingData);
        qint64 contentLength = r1->header(QNetworkRequest::ContentLengthHeader).toLongLong();
        SandboxReply*  r2 = new SandboxReply(this,r1);

        qDebug() << "===================================";
        qDebug() << r1->bytesAvailable() << "/" << r1->size();
        qDebug() << r2->bytesAvailable() << "/" << r2->size();
        qDebug() << "===================================";

        this->pageLoadingStarted = false;
        return r2;
    }
*/

    return  QNetworkAccessManager::createRequest(op,r,outgoingData);
}



void
MyNetworkAccessManager::activateMixedContentBlocking(bool state)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "MCB is now " << state;

    this->MixedContentBlockerActive = state;
}

void
MyNetworkAccessManager::URLbarChanged(const QString& url)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->URL = url;
}

void
MyNetworkAccessManager::loadStarted()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->pageLoadingStarted = true;
}
