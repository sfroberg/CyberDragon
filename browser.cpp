/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */


#include <QWebElement>
#include <QWebElementCollection>
#include <QMessageBox>
#include <QDialogButtonBox>
#include <QAuthenticator>
#include <QUuid>
#include <QWebHistory>
#include <QWebPluginFactory>


#include "browser.h"
#include "ui_browser.h"

#include "general.h"
#include "ui_general.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "trackerblocker.h"
#include "ui_trackerblocker.h"

#include "cookie.h"
#include "ui_cookie.h"

#include "encryption.h"
#include "ui_encryption.h"

#include "cache.h"
#include "ui_cache.h"

#include "proxy.h"
#include "ui_proxy.h"

#include "log.h"
#include "ui_log.h"

#include "serverinfo.h"
#include "ui_serverinfo.h"

#include <QSslCipher>
#include <QSslKey>
#include <QTextEdit>

#include <QAbstractNetworkCache>

#include "colors.h"


Browser::Browser(QWidget *parent, const QString& url) :
    QWidget(parent),serverInfo(NULL),
    ui(new Ui::Browser)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);

    this->ui->Ok_pushButton->setFixedWidth(100);
    this->mainWindow = qobject_cast<MainWindow*>(parent);
    this->ui->webView->setMainWindow(this->mainWindow);


    this->cookieJar = new MyCookieJar;
    this->cookieJar->setMainWindow(this->mainWindow);
    this->manager = new MyNetworkAccessManager;

    Q_ASSERT(this->mainWindow->logTab != NULL);
    connect(this->manager,SIGNAL(dataReady(QStringList,QColor)),this->mainWindow->logTab,SLOT(dataReady(QStringList,QColor)));


    this->manager->setBrowser(this);
    this->manager->setMainWindow(this->mainWindow);

    Q_ASSERT(this->mainWindow->cacheTab != NULL);
    QNetworkDiskCache*   disk = new QNetworkDiskCache;
    disk->setCacheDirectory(this->mainWindow->cacheTab->ui->cacheDir_lineEdit->text());
    disk->setMaximumCacheSize(this->mainWindow->cacheTab->ui->cacheSize_spinBox->value());
    this->manager->setCache(disk);


    this->manager->setCookieJar(this->cookieJar);

    this->page = new MyWebPage(this->mainWindow);
    this->page->setNetworkAccessManager(this->manager);
    this->ui->webView->setPage(this->page);


    connect(this->ui->webView,SIGNAL(urlChanged(QUrl)),this,SLOT(slot_urlChanged(QUrl)));

    /* Delegate all these signals to upwards ... (aka all the various child elements signals
     * that this Browser widget owns) */
    connect(this->ui->webView->page(),SIGNAL(linkHovered(QString,QString,QString)),this,SIGNAL(linkHovered(QString,QString,QString)));
    connect(this->ui->webView,SIGNAL(titleChanged(QString)),this,SIGNAL(titleChanged(QString)));
    connect(this->ui->webView,SIGNAL(loadStarted()),this,SIGNAL(loadStarted()));
    connect(this->ui->webView,SIGNAL(urlChanged(QUrl)),this,SIGNAL(urlChanged(QUrl)));
    connect(this->ui->webView->page(),SIGNAL(unsupportedContent(QNetworkReply*)),this,SIGNAL(unsupportedContent(QNetworkReply*)));



    connect(this->ui->webView,SIGNAL(loadProgress(int)),this->ui->URL_progressBar,SLOT(setValue(int)));
    connect(this->ui->webView,SIGNAL(loadFinished(bool)),this,SLOT(loadFinished(bool)));
    connect(this->ui->URL_comboBox->lineEdit(),SIGNAL(returnPressed()),this,SLOT(returnPressed()));
    connect(this->ui->Go_pushButton,SIGNAL(clicked()),this,SLOT(returnPressed()));
    connect(this->ui->webView,SIGNAL(loadStarted()),this,SLOT(slot_loadStarted()));
    connect(this->ui->webView->page()->mainFrame(),SIGNAL(loadStarted()),this,SLOT(slot_frame_loadStarted()));
    connect(this->ui->webView->page()->mainFrame(),SIGNAL(loadStarted()),manager,SLOT(loadStarted()));
    connect(this->ui->webView->page()->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(slot_frame_loadFinished(bool)));


    QKeySequence keys_home(Qt::ALT + Qt::Key_Home);
    this->ui->Home_pushButton->setShortcut(keys_home);

    /* Setting F5 key to reload webView */
    QKeySequence keys_refresh(QKeySequence::Refresh);  //or (Qt::Key_F5), doesn't matter
    this->ui->Reload_pushButton->setShortcut(keys_refresh);

    /* Backspace */
    QKeySequence keys_back(Qt::Key_Backspace);
    this->ui->Previous_pushButton->setShortcut(keys_back);
    connect(this->ui->Previous_pushButton,SIGNAL(clicked()),this,SLOT(slot_backward()));

    /* Shift + Backspace */
    QKeySequence keys_forward(Qt::SHIFT + Qt::Key_Backspace);
    this->ui->Next_pushButton->setShortcut(keys_forward);
    connect(this->ui->Next_pushButton,SIGNAL(clicked()),this,SLOT(slot_forward()));


    /* F6, switch between URL bar and webview */
    QKeySequence keys_url(Qt::Key_F6);
    QAction*    actionUrl = new QAction(this->ui->URL_comboBox->lineEdit());
    actionUrl->setShortcut(keys_url);
    this->ui->URL_comboBox->lineEdit()->addAction(actionUrl);
    connect(actionUrl,SIGNAL(triggered()),this,SLOT(urlFocus()));

    /* Esc */
    QKeySequence keys_abort_connection(Qt::Key_Escape);
    QAction*    actionAbortConnection = new QAction(this->ui->webView);
    actionAbortConnection->setShortcut(keys_abort_connection);
    this->ui->webView->addAction(actionAbortConnection);
    connect(actionAbortConnection,SIGNAL(triggered()),this->ui->webView,SLOT(stop()));

    /* =============================================== */
    this->page->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(this->page,SIGNAL(linkClicked(QUrl)),this,SLOT(handlelinkClicked(QUrl)));
    /* =============================================== */



    connect(this,SIGNAL(activateMixedContentBlocking(bool)),this->manager,SLOT(activateMixedContentBlocking(bool)));
    connect(this->manager,SIGNAL(numberOfTrackers(int)),this,SLOT(updateTrackersNumber(int)));
    connect(this->manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(checkTrackers(QNetworkReply*)));
    connect(this->manager,SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),this,SLOT(authenticate(QNetworkReply*,QAuthenticator*)));

    /* ========================================================= */
    connect(this->manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(finished(QNetworkReply*)));
    connect(this->manager,SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)),this,SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
    connect(this->manager,SIGNAL(finished(QNetworkReply*)),this,SLOT(checkEncrypted(QNetworkReply*)));
    /* ========================================================= */

    connect(this->ui->URL_comboBox->lineEdit(),SIGNAL(textChanged(QString)),this->manager,SLOT(URLbarChanged(QString)));


    this->serverInfo = new ServerInfo(this->mainWindow);    // Initialize Server Info tab
    this->ui->URL_comboBox->lineEdit()->setText(url);
    this->ui->webView->load(url);
}

Browser::~Browser()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    delete ui;
}

MyCookieJar*
Browser::getCookieJar()const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->cookieJar;
}

void
Browser::slot_loadStarted()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    emit activateMixedContentBlocking(false);
}

void
Browser::slot_frame_loadStarted()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->Ok_pushButton->setIcon(QIcon(QLatin1String(":/icons/64x64/empty-64x64.png")));
    this->ui->Ok_pushButton->setText(QLatin1String("          "));

    emit clearTrackerList();
    emit clearMixedContentBlockingList();

    Q_ASSERT(this->mainWindow->cookieTab != NULL);
    if(this->mainWindow->cookieTab->ui->ClearCookieListOnPageLoad_checkBox->isChecked()) {
        emit clearCookieList();
    }

    Q_ASSERT(this->mainWindow->proxyTab != NULL);
    if(this->mainWindow->proxyTab->ui->ProxyHoppingPageLoad_radioButton->isChecked()) {
        this->mainWindow->proxyTab->randomProxy();
    }

    Q_ASSERT(this->mainWindow->generalTab != NULL);
    /* User-Agent Spoofing */
    if(this->mainWindow->generalTab->ui->UserAgent_treeWidget->topLevelItemCount() > 0 && this->mainWindow->generalTab->ui->UserAgentRandomly_radioButton->isChecked()) {
        this->mainWindow->generalTab->randomUserAgent();
    }


    /* OBS! this->ui->webView->page()->mainFrame()->requestedUrl() contains the requested URL.
     * Long before even first QNetworkRequest has been sended!
     *
     * So you could use it, for example, in MyNetworkAccessManager::createRequest() function
     * if you ever need to compare the requested URL and the *actual* URL of the first QNetworkRequest */
}

MyNetworkAccessManager*
Browser::getNetworkAccessManager() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->manager != NULL);

    return  this->manager;
}


QWebFrame*
Browser::getWebFrame() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->ui->webView->page()->mainFrame();
}

QWebPage*
Browser::getWebPage() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->ui->webView->page();
}

QWebView*
Browser::getWebView() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->ui->webView;
}

QString
Browser::getTitle() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  this->ui->webView->title();
}


void
Browser::loadFinished(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ... " << this->ui->webView->url();

    this->ui->URL_progressBar->setValue(0);
}

void
Browser::returnPressed()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->encryptionTab != NULL);


    QWebHistory*    history = this->ui->webView->page()->history();
    QWebHistoryItem goToItem = history->itemAt(this->ui->URL_comboBox->currentIndex());

    qDebug() << "COMBOBOX INDEX " << this->ui->URL_comboBox->currentIndex();
    qDebug() << "HISTORY ITEM ON " << goToItem.url().toDisplayString();

    /* First we check if URL is already in history.
     * If it is then go directly there and skip the rest */
    if(goToItem.isValid() && this->historyList.contains(goToItem)) {
            qDebug() << "URL IS IN HISTORY";
            this->mainURL = this->ui->URL_comboBox->lineEdit()->text();
            history->goToItem(goToItem);
    } else {
        qDebug() << "URL IS NOT IN HISTORY";

        this->mainURL = this->ui->URL_comboBox->lineEdit()->text();

        qDebug() << "=======================================";
        qDebug() << "MAINURL IS " << this->mainURL;
        qDebug() << "=======================================";

        /* User did not give the scheme part. Autofill with http:// and with https://
         * if HTTP Enforcing has been enabled */
        if(this->mainURL.scheme().isEmpty()) {
            if(this->mainWindow->encryptionTab->ui->HTTPSEnforcing_checkBox->isChecked()) {
                this->mainURL = QLatin1String("https://") + this->mainURL.toString();
            } else {
                this->mainURL = QLatin1String("http://") + this->mainURL.toString();
            }
        }

        qDebug() << "=======================================";
        qDebug() << "Changed URL " << this->mainURL;

        this->ui->webView->load(this->mainURL);

    }
    this->urlFocus();
}

QUrl
Browser::getUrl()const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  QUrl(this->ui->URL_comboBox->lineEdit()->text());
}



inline uint qHash(const QWebHistoryItem& key, uint seed = 0)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return qHash(key.url().toDisplayString(),seed);
}

inline  bool    operator==(const QWebHistoryItem& a, const QWebHistoryItem& b)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return  (a.url() == b.url());
}

void
Browser::slot_urlChanged(const QUrl& url)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ... " << url;

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->cookieTab != NULL);
    Q_ASSERT(this->mainWindow->generalTab != NULL);

    /* Disabling plugins from general tab should also prevent JavaScript plugins enumeration.
     * But it doesn't! So we force it here (Bug filed to Qt) */
    if(!this->mainWindow->generalTab->ui->Plugins_checkBox->isChecked()) {
        /* This will set the window.navigator.plugins to NULL value and prevent plugins enumeration */
        QString x = "window.navigator = null;";
        this->ui->webView->page()->mainFrame()->evaluateJavaScript(x);
    }



    if(url.scheme() == QLatin1Literal("http")) {
        emit cipherDetailsChanged(QLatin1String(""));
    }

    QWebHistory*    history = this->ui->webView->page()->history();

    this->ui->Next_pushButton->setEnabled(history->canGoForward());
    this->ui->Previous_pushButton->setEnabled(history->canGoBack());

    QList<QWebHistoryItem>  items = history->items();
    if(!items.isEmpty()) {
        this->ui->URL_comboBox->clear();
        this->historyList.clear();
        foreach(const QWebHistoryItem& item,items) {
            this->ui->URL_comboBox->addItem(item.url().toDisplayString());
            this->historyList.insert(item);
        }
    }

    this->manager->zeroTrackerCount();
    this->ui->URL_comboBox->lineEdit()->setText(url.toString());
    this->ui->URL_comboBox->lineEdit()->setCursorPosition(0);


    if(this->mainWindow->cookieTab->ui->ClearCookieListOnPageLoad_checkBox->isChecked()) {
        /* Ugly, replace this with signal that either MainWindow or TabbedBrowser catches and handles ... */
        this->mainWindow->cookieTab->ui->ClearCookieList_pushButton->click();
    }

    this->ui->lcdNumber->display(0);
    this->ui->Ok_pushButton->setIcon(QIcon(QLatin1String(":/empty-64x64.png")));

    if(this->ui->webView->url().scheme() == QLatin1String("https")) {
        this->ui->encrypted_pushButton->setIconSize(QSize(16,16));
        this->ui->encrypted_pushButton->setIcon(QIcon(QLatin1String(":/icons/16x16/document-encrypt.png")));
        this->ui->encrypted_pushButton->setToolTip(QLatin1String("Encrypted"));
    }
    if(this->ui->webView->url().scheme() == QLatin1String("http")) {
        this->ui->encrypted_pushButton->setIconSize(QSize(16,16));
        this->ui->encrypted_pushButton->setIcon(QIcon(QLatin1String(":/icons/64x64/empty-64x64.png")));
        this->ui->encrypted_pushButton->setToolTip(QLatin1String("Unencrypted"));
    }

    /* MCB */
    if(this->ui->webView->url().scheme() == QLatin1String("https")) {
        emit activateMixedContentBlocking(true);
    } else {
        emit activateMixedContentBlocking(false);
    }

 /* ========================================== */
    if(!url.host().isEmpty() && this->mainWindow->ui->serverInfoLookup_checkBox->isChecked()) {
          QHostInfo::lookupHost(url.host(),this->serverInfo,SLOT(lookUp(QHostInfo)));
    }
}




void Browser::on_Home_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->generalTab != NULL);

    this->ui->webView->load(QUrl(this->mainWindow->generalTab->ui->HomePage_lineEdit->text()));

}


void
Browser::urlFocus()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    static  bool    selected = false;

    selected ^= 0x1;

    if(!selected) {
        this->ui->webView->setFocus();
    } else {
        this->ui->URL_comboBox->lineEdit()->selectAll();
        this->ui->URL_comboBox->lineEdit()->setFocus();
    }

}


void
Browser::updateTrackersNumber(int n)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->lcdNumber->display(n);
}



void
Browser::handlelinkClicked(const QUrl & url)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->encryptionTab != NULL);

    qDebug() << "==============================================";
    qDebug() << "Clicked link " << url;
    qDebug() << "==============================================";

    QUrl    tmp(url);

    if(this->mainWindow->encryptionTab->ui->HTTPSEnforcing_checkBox->isChecked() && url.scheme() == QLatin1String("http")) {
        tmp.setScheme(QLatin1String("https"));

        qDebug() << "Replacing it to " << tmp;
        qDebug() << "==============================================";
    }
    this->mainURL = tmp;
    this->ui->webView->page()->currentFrame()->load(tmp);
}


void
Browser::checkTrackers(QNetworkReply* r)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(r != NULL);
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->trackerblockerTab != NULL);


    /* To handle deleteLater() automatically, without us needing to worry when to call it */
    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(r, &QObject::deleteLater);


    if(this->mainWindow->trackerblockerTab->ui->BlockTrackers_checkBox->isChecked() && reply) {
            if(this->ui->lcdNumber->value() == 0) {
                this->ui->Ok_pushButton->setIcon(QIcon(QLatin1String(":/icons/64x64/thumbs-up-64x64.png")));
                this->ui->Ok_pushButton->setText(QLatin1String("CLEAN!"));
                this->ui->Ok_pushButton->setToolTip(QLatin1String("No trackers were found :-)"));
            } else {
                this->ui->Ok_pushButton->setIcon(QIcon(QLatin1String(":/icons/64x64/thumbs-down-64x64.png")));
                this->ui->Ok_pushButton->setText(QLatin1String("NOT CLEAN!"));
                this->ui->Ok_pushButton->setToolTip(QString::number(this->ui->lcdNumber->value()) + QLatin1String(" trackers found!"));
            }
        } else {
            this->ui->Ok_pushButton->setIcon(QIcon(QLatin1String(":/icons/64x64/empty-64x64.png")));
            this->ui->Ok_pushButton->setText(QLatin1String("          "));
            this->ui->Ok_pushButton->setToolTip(QLatin1String("Tracker blocker disabled!"));
    }
}

void
Browser::slot_frame_loadFinished(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->generalTab != NULL);
    Q_ASSERT(this->mainWindow->encryptionTab != NULL);

    /* Loading was ok */
    if(ok) {
/*******************************************************************/
/* Experimental, not tested stuff. Uncomment if you want to try them */
/*******************************************************************/

    /* Optional window.name JavaScript tracking prevention.
     * Comment out for now untill figuring if this is the right place to put this
     * and when this has been tested more */
    /*{
        QString x = "window.name = \"\";";
        this->ui->webView->page()->mainFrame()->evaluateJavaScript(x);
    }*/


    /* Optional window.navigator plugins JavaScript tracking prevention.
     * Comment out for now untill figuring if this is the right place to put this
     * and when this has been tested more. Please also see slot_urlChanged(const QUrl&) */
    /* {
        QString x = "window.navigator = null;";
        this->ui->webView->page()->mainFrame()->evaluateJavaScript(x);
     }*/

        if(this->mainWindow->generalTab->ui->PreventGoogleTrackingRedirectLinks_checkBox->isChecked()) {
            QWebElement document = this->ui->webView->page()->mainFrame()->documentElement();
            QWebElementCollection   links = document.findAll(QLatin1String("a"));
            foreach(QWebElement link,links) {
                QString href = link.attribute(QLatin1String("href"));
                if(!href.isEmpty()) {
                    href.replace(QRegularExpression(QLatin1String("/url\\?q=(https?\\://.*)&sa=.*")),QLatin1String("\\1"));
                    if(this->mainWindow->encryptionTab->ui->HTTPSEnforcing_checkBox->isChecked()) {
                        href.replace(QRegularExpression(QLatin1String("^http:")),QLatin1String("https:"));
                    }
                    link.setAttribute(QLatin1String("href"),QUrl::fromPercentEncoding(href.toUtf8()));
                }
            }
        }
    }


}



void
Browser::sslErrors(QNetworkReply * r, const QList<QSslError> & errors)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(r != NULL);

    /* To handle deleteLater() automatically, without us needing to worry when to call it */
    QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(r, &QObject::deleteLater);

    if(reply) {

        QString e = "The following SSL errors occured:\n\n";
        foreach(const QSslError& error,errors) {

            qDebug() << (int)error.error() << "\t" << error.errorString();

            e += QLatin1String("\t") + error.errorString() + QLatin1String("\n");
            switch(error.error()) {
                case    QSslError::SelfSignedCertificate:
                {
                    e += QLatin1String("\nDo you still want to continue?");
                    QMessageBox::StandardButton ret = QMessageBox::warning(this,QLatin1String("SSL Error!"),e,QMessageBox::Yes | QMessageBox::No);
                    if(ret == QMessageBox::Yes)
                        reply->ignoreSslErrors();
                }
                break;
                default:
                {
                    QUrl    url = reply->url();
                    url.setScheme(QLatin1String("http"));
                    this->ui->webView->load(url);
                }
                break;
            }
        }
    }
}


void
Browser::finished(QNetworkReply * reply)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(reply != NULL);
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->encryptionTab != NULL);
    Q_ASSERT(this->mainWindow->logTab != NULL);

    qDebug() << reply->url();

    if(!reply->url().isEmpty()) {
        if(reply->error() == QNetworkReply::NoError) {
            qDebug() << reply->url()   << " OK!";                           
        } else {

            qDebug() << reply->url() << " " << reply->errorString() << " (" << reply->error() << ") ";

            if(reply->url().scheme() == QLatin1String("https") && this->mainWindow->encryptionTab->ui->HTTPSEnforcing_checkBox->isChecked()) {

                qDebug() << reply->url() << " I tried HTTPS but there were some problems. ERROR: " << reply->errorString() << " (" << (int)reply->error() << ")";
                qDebug() << reply->url();
                qDebug() << this->mainURL;

                if(reply->url().host() == this->mainURL.host()) {
                    QUrl    url = reply->url();
                    url.setScheme(QLatin1String("http"));
                    this->ui->webView->load(url);
                }
            }
        }
    }

    if(this->mainWindow->logTab->ui->enableLog_checkBox->isChecked() && !reply->url().isEmpty()) {
        QStringList data;
        int httpStatusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        data << QDateTime::currentDateTime().toString();
        data << (QString::number(httpStatusCode) + QLatin1String(" ") + reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString());
        data << reply->rawHeader("Content-Type");
        data << reply->url().toDisplayString();

        if(httpStatusCode < 200) {
            emit dataReady(data,QColor(0,0,0));
        }
        if(httpStatusCode == 200) {
            emit dataReady(data,MediumSpringGreen);
        }
        if(httpStatusCode > 200 && httpStatusCode < 400) {
            emit dataReady(data,QColor(0,0,0));
        }
        if(httpStatusCode >= 400) {
           emit dataReady(data,ChilliPepper);
        }
    }
}



/* ======================================== */
void
Browser::checkEncrypted(QNetworkReply* reply)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(reply != NULL);

    if(reply->url() == this->ui->webView->url() && reply->url().scheme() == QLatin1String("https") && this->ui->webView->url().scheme() == QLatin1String("https")) {

        QSslConfiguration   sslConfig = reply->sslConfiguration();
        if(!sslConfig.isNull()) {

            QSslCipher          sessionCipher = sslConfig.sessionCipher();
            QString text = "<pre>";

        if(!sessionCipher.isNull()) {


            text += reply->url().host() + "<br>";
            text += "<br><b>Cipher Info</b><br>";
            text += "_____________________________________________________________<br>";
            text += "Cipher:                         " + \
                       sessionCipher.name() + "<br>Protocol:                       " + \
                       sessionCipher.protocolString() + "<br>Authentication:                 " + \
                       sessionCipher.authenticationMethod() + "<br>Encryption:                     " + \
                       sessionCipher.encryptionMethod() + "<br>Key exchange:                   " + \
                       sessionCipher.keyExchangeMethod() + "<br>Bits (used/supported):          " + \
                       QString::number(sessionCipher.usedBits()) + "/" + QString::number(sessionCipher.supportedBits());
         } else {

            qDebug() << "======================================";
            qDebug() << "CIPHERS";
            qDebug() << sslConfig.ciphers();
            qDebug() << "======================================";

            text += "<b style=\"color:red\">!!!WARNING!!! NO CIPHER FOUND! POSSIBLY USING NULL CIPHER!!!</b>";            
        }

        QSslCertificate cert = sslConfig.peerCertificate();

        if(!cert.isNull()) {
            text += "<br><br><b>Certificate Info</b><br>";
            text += "_____________________________________________________________<br>";

            text += "Fingerprint:                    ";
            QByteArray   digest = cert.digest(QCryptographicHash::Sha1).toHex().toUpper();

            QString s(digest.at(0));
            s += QString(digest.at(1));
            register size_t  n = digest.count();
            for(register unsigned int i = 2;i < n;i+=2) {
                s += ":" + QString(digest.at(i)) + QString(digest.at(i+1));
            }
            text += s;
            text += "<br>Version:                        " + cert.version();
            text += "<br>Serial Number:                  " + cert.serialNumber();
            text += "<br>Effective date:                 " + cert.effectiveDate().toString();
            text += "<br>Expiry date:                    " + cert.expiryDate().toString();
            text += "<br>Blacklisted:                    " + (cert.isBlacklisted() ? QString("Yes") : QString("No")) + "\n";                        
            text += "<br><b>Issuer Info</b>";

            if(!cert.issuerInfo(QSslCertificate::Organization).isEmpty())
                text += "<br>Organization (O):               " + cert.issuerInfo(QSslCertificate::Organization).join("");

            if(!cert.issuerInfo(QSslCertificate::CommonName).isEmpty())
                text += "<br>Common Name (CN):               " + cert.issuerInfo(QSslCertificate::CommonName).join("");

            if(!cert.issuerInfo(QSslCertificate::LocalityName).isEmpty())
                text += "<br>Locality Name (L):              " + cert.issuerInfo(QSslCertificate::LocalityName).join("");

            if(!cert.issuerInfo(QSslCertificate::OrganizationalUnitName).isEmpty())
                text += "<br>Organizational Unit Name (OU):  " + cert.issuerInfo(QSslCertificate::OrganizationalUnitName).join("");

            if(!cert.issuerInfo(QSslCertificate::CountryName).isEmpty())
                text += "<br>Country Name (C):               " + cert.issuerInfo(QSslCertificate::CountryName).join("");

            if(!cert.issuerInfo(QSslCertificate::StateOrProvinceName).isEmpty())
                text += "<br>State or Province (ST):         " + cert.issuerInfo(QSslCertificate::StateOrProvinceName).join("");

            if(!cert.issuerInfo(QSslCertificate::DistinguishedNameQualifier).isEmpty())
                text += "<br>Distinguished Name Qualifier:   " + cert.issuerInfo(QSslCertificate::DistinguishedNameQualifier).join("");

            if(!cert.issuerInfo(QSslCertificate::SerialNumber).isEmpty())
                text += "<br>Certificate's Serial Number:    " + cert.issuerInfo(QSslCertificate::SerialNumber).join("");

            if(!cert.issuerInfo(QSslCertificate::EmailAddress).isEmpty())
                text += "<br>Email address:                  " + cert.issuerInfo(QSslCertificate::EmailAddress).join("");


            text += "<br><br><b>Subject Info</b>";

            if(!cert.subjectInfo(QSslCertificate::Organization).isEmpty())
                text += "<br>Organization (O):               " + cert.subjectInfo(QSslCertificate::Organization).join("");

            if(!cert.subjectInfo(QSslCertificate::CommonName).isEmpty())
                text += "<br>Common Name (CN):               " + cert.subjectInfo(QSslCertificate::CommonName).join("");

            if(!cert.subjectInfo(QSslCertificate::LocalityName).isEmpty())
                text += "<br>Locality Name (L):              " + cert.subjectInfo(QSslCertificate::LocalityName).join("");

            if(!cert.subjectInfo(QSslCertificate::OrganizationalUnitName).isEmpty())
                text += "<br>Organizational Unit Name (OU):  " + cert.subjectInfo(QSslCertificate::OrganizationalUnitName).join("");

            if(!cert.subjectInfo(QSslCertificate::CountryName).isEmpty())
                text += "<br>Country Name (C):               " + cert.subjectInfo(QSslCertificate::CountryName).join("");

            if(!cert.subjectInfo(QSslCertificate::StateOrProvinceName).isEmpty())
                text += "<br>State or Province (ST):         " + cert.subjectInfo(QSslCertificate::StateOrProvinceName).join("");

            if(!cert.subjectInfo(QSslCertificate::DistinguishedNameQualifier).isEmpty())
                text += "<br>Distinguished Name Qualifier:   " + cert.subjectInfo(QSslCertificate::DistinguishedNameQualifier).join("");

            if(!cert.subjectInfo(QSslCertificate::SerialNumber).isEmpty())
                text += "<br>Certificate's Serial Number:    " + cert.subjectInfo(QSslCertificate::SerialNumber).join("");

            if(!cert.subjectInfo(QSslCertificate::EmailAddress).isEmpty())
                text += "<br>Email address:                  " + cert.subjectInfo(QSslCertificate::EmailAddress).join("");

            qDebug() << "=========================";
            qDebug() << cert.toText();
            qDebug() << "=========================";

            QSslKey key = cert.publicKey();

            if(!key.isNull()) {
                text += "<br><br><b>Public Key Info (";
                text += QString::number(key.length()) + " bit ";
                switch(key.algorithm()) {
                    case    QSsl::Rsa:
                        text += "RSA";
                    break;
                    case    QSsl::Dsa:
                    text += "DSA";
                    break;
                }
                text += ")</b><br>";
                text += "_____________________________________________________________<br>";
                text += key.toPem();
            }
        }

        text += "</pre>";
        emit cipherDetailsChanged(text);

        }
    }
}



QString
Browser::getCache() const
{
    qDebug() << __PRETTY_FUNCTION__ ;

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->cacheTab != NULL);
    Q_ASSERT(this->manager != NULL);

    QNetworkDiskCache* diskCache = qobject_cast<QNetworkDiskCache*>(this->manager->cache());

    if(diskCache) {
        if(this->mainWindow->cacheTab->ui->noCache_radioButton->isChecked()) {        
                return "Disk cache disabled";
        }
        if(this->mainWindow->cacheTab->ui->allwaysCache_radioButton->isChecked()) {
            return "Always use disk cache (Offline mode)\n\nCache Directory: " + diskCache->cacheDirectory() + "\nMax Cache Size: " + \
                    QString::number(diskCache->maximumCacheSize()) + " MB";
        }
        if(this->mainWindow->cacheTab->ui->enableCache_radioButton->isChecked()) {
                return "Disk cache enabled\n\nCache Directory: " + diskCache->cacheDirectory() + "\nMax Cache Size: " + \
                    QString::number(diskCache->maximumCacheSize()) + " MB";
        }
    }
    return  QString();
}


void
Browser::setCache()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->cacheTab != NULL);
    Q_ASSERT(this->manager != NULL);

    QNetworkDiskCache* diskCache = qobject_cast<QNetworkDiskCache*>(this->manager->cache());

    if(diskCache) {

        diskCache->setCacheDirectory(this->mainWindow->cacheTab->ui->cacheDir_lineEdit->text());
        diskCache->setMaximumCacheSize(this->mainWindow->cacheTab->ui->cacheSize_spinBox->value());

    }
}

void
Browser::clearCache()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->manager != NULL);

    QNetworkDiskCache* diskCache = qobject_cast<QNetworkDiskCache*>(this->manager->cache());

    if(diskCache) {
        diskCache->clear();
    }
}



MainWindow*
Browser::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);

    return  this->mainWindow;
}

void
Browser::setMainWindow(MainWindow* m)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->mainWindow != NULL);

    this->mainWindow = m;
}


void
Browser::slot_forward()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(this->ui->webView->page()->history()->canGoForward()) {
        this->ui->webView->forward();
    }

}

void
Browser::slot_backward()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(this->ui->webView->page()->history()->canGoBack()) {
        this->ui->webView->back();
    }

}



void
Browser::authenticate(QNetworkReply * reply, QAuthenticator * authenticator)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QDialog* dialog = new QDialog(QApplication::activeWindow());
    dialog->setWindowTitle(QLatin1String("HTTP Authentication"));

    QGridLayout* layout = new QGridLayout(dialog);
    dialog->setLayout(layout);

    QLabel* messageLabel = new QLabel(dialog);
    messageLabel->setWordWrap(true);
    QString messageStr = QString("Enter with username and password for: %1");
    messageLabel->setText(messageStr.arg(reply->url().toString()));
    layout->addWidget(messageLabel, 0, 1);

    QLabel* userLabel = new QLabel(QLatin1String("Username:"), dialog);
    layout->addWidget(userLabel, 1, 0);
    QLineEdit* userInput = new QLineEdit(dialog);
    layout->addWidget(userInput, 1, 1);

    QLabel* passLabel = new QLabel(QLatin1String("Password:"), dialog);
    layout->addWidget(passLabel, 2, 0);
    QLineEdit* passInput = new QLineEdit(dialog);
    passInput->setEchoMode(QLineEdit::Password);
    layout->addWidget(passInput, 2, 1);

    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok| QDialogButtonBox::Cancel, Qt::Horizontal, dialog);
    connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));
    layout->addWidget(buttonBox, 3, 1);



    if (dialog->exec() == QDialog::Accepted) {
        authenticator->setUser(userInput->text());
        authenticator->setPassword(passInput->text());
    }

    delete dialog;
}

ServerInfo*
Browser::getServerInfo()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity checks */
    Q_ASSERT(this->serverInfo != NULL);

    return  this->serverInfo;
}
