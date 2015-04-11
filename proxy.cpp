/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "proxy.h"
#include "ui_proxy.h"

#include "notification.h"
#include "ui_notification.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QColor>
#include <QWebElement>
#include <QWebElementCollection>
#include <QWebFrame>
#include <QFileDialog>
#include <QThread>
#include <QSettings>

/* Helper function for random proxy hopping */
inline static
quint32 random(quint32 low, quint32 high)
{

    return  (qrand() % ((high + 1) - low) + low);
}

quint32 Proxy::proxy_list_counter = 0;
quint32 Proxy::total_number_of_proxies = 0;
quint32 Proxy::total_number_of_working_proxies = 0;
qint64  Proxy::NUMBER_OF_THREADS = 4;
quint32 Proxy::proxy_checker_timeout = 30000;
quint32 Proxy::NUMBER_OF_PROXY_SITES = 8;


quint32 Proxy::NUMBER_OF_PROXIES = 0;
quint32 Proxy::OFFLINE_PROXIES = 0;

/* FIXME/ADD:
 * - Better (working, more robust, fast) crash free proxy checker.
 * - Better (more GENERAL, handle multiple html pages when parsing, more robust, remove duplicate code)
 *   proxy fetcher.
 * - SOCKS5 proxy support for proxy fetcher & checker
     (now only can set SOCKS5 proxy manually and not by clicking from proxy list)
 * - Autoload proxy list.
 * - Automatic checking of dead proxies and remove (or disable) them from list
     (GUI widgets: checkbox for enable/disable automatic check and spinbox for time between checks)
 * - Add custom proxy rules (regular expressions) that determine when to use proxy (and what proxy)
     and when to use direct connection. Similar to FoxyProxy Firefox addon.
*/

Proxy::Proxy(MainWindow *m, QWidget *parent) :
    QWidget(parent),
    settingsFile(QLatin1String("config.ini")), mainWindow(m),gi(NULL),worker(NULL),workerThread(NULL),ui(new Ui::Proxy)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);

    connect(&this->proxyHoppingTimer,SIGNAL(timeout()),this,SLOT(randomProxy()));

    /* Initially make both proxy fetcher and checker controls invisible.
     * Make them visible only when corresponding button (a.k.a. either "Get proxies" or "Check proxies")
     * has been pressed */
    this->ui->ProxyFetching_label->setVisible(false);
    this->ui->ProxyFetching_progressBar->setVisible(false);
    this->ui->ProxyFetching_progressBar->setMaximum(Proxy::NUMBER_OF_PROXY_SITES);

    this->ui->ProxyChecking_label->setVisible(false);
    this->ui->ProxyChecking_progressBar->setVisible(false);
    this->ui->stopChecking_pushButton->setVisible(false);

    /* Random Proxy hopping controls */
        this->proxyHopping = new QButtonGroup(this);
        this->proxyHopping->addButton(this->ui->NoRandomProxyHopping_radioButton,RandomProxyHopping::NoProxyHopping);
        this->proxyHopping->addButton(this->ui->ProxyHoppingTimed_radioButton,RandomProxyHopping::ProxyHoppingTimed);
        this->proxyHopping->addButton(this->ui->ProxyHoppingPageLoad_radioButton,RandomProxyHopping::ProxyHoppingRequest);
        connect(this->proxyHopping,SIGNAL(buttonClicked(int)),this,SLOT(proxyHoppingbuttonClicked(int)));
        connect(&this->proxyHoppingTimer,SIGNAL(timeout()),this,SLOT(randomProxy()));

    /* Proxy fetcher & checker stuff */
        connect(this,SIGNAL(readyForProxyChecking()),this,SLOT(proxyListReady()));

        this->ui->ProxyFetcherChecker_treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

        action_removeProxy = new QAction(tr("Remove proxy ..."),this->ui->ProxyFetcherChecker_treeWidget);
        connect(action_removeProxy,SIGNAL(triggered()),this,SLOT(removeProxy()));
        this->ui->ProxyFetcherChecker_treeWidget->addAction(action_removeProxy);

        QAction*    action_addBlacklistedProxy = new QAction(tr("Add proxy to blacklist ..."),this->ui->ProxyFetcherChecker_treeWidget);
        connect(action_addBlacklistedProxy,SIGNAL(triggered()),this,SLOT(on_AddBlacklistedProxies_pushButton_clicked()));
        this->ui->ProxyFetcherChecker_treeWidget->addAction(action_addBlacklistedProxy);

        /* It is possible to use QWebPage for html parsing.
         * Without GUI for them ! :-)
         * Actual proxy loading is done in Proxy::on_GetProxies_pushButton_clicked() */

        this->proxy_freeproxylists = new QWebPage(this);
        this->proxy_free_proxy_list = new QWebPage(this);
        this->proxy_xroxy = new QWebPage(this);
        this->proxy_xroxy1 = new QWebPage(this); // <---- Shit. Make general, multipage proxy fetcher
        this->proxy_xroxy2 = new QWebPage(this); // <--- Ditto
        this->proxy_samair = new QWebPage(this);
        this->proxy_coolproxy = new QWebPage(this);
        this->proxy_proxy_list = new QWebPage(this);

        /* Register proxy parsers that are each fired when each QWebPage has finished html page loading */
        connect(this->proxy_freeproxylists->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(freeproxylist(bool)));
        connect(this->proxy_samair->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(samair(bool)));
        connect(this->proxy_coolproxy->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(coolproxy(bool)));
        connect(this->proxy_free_proxy_list->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(free_proxy_list(bool)));

        connect(this->proxy_xroxy->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(xroxy(bool)));
        connect(this->proxy_xroxy1->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(xroxy1(bool)));
        connect(this->proxy_xroxy2->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(xroxy2(bool)));
        connect(this->proxy_proxy_list->mainFrame(),SIGNAL(loadFinished(bool)),this,SLOT(proxy_list(bool)));
        connect(this,SIGNAL(allProxiesFinished()),this,SLOT(slot_allProxiesFinished()));

        this->loadSettings();
        this->ui->ProxyFetcherChecker_groupBox->adjustSize();
}

Proxy::~Proxy()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->saveSettings();
    delete ui;
}

MainWindow*
Proxy::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->mainWindow;
}

void
Proxy::on_ClearProxy_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->Proxy_lineEdit->clear();
    this->ui->Proxy_spinBox->setValue(80);
    this->ui->ProxyUsername_lineEdit->clear();
    this->ui->ProxyPassword_lineEdit->clear();

    this->ui->ProxyFetcherChecker_treeWidget->clearSelection();
    this->getMainWindow()->ui->currentProxy_label->clear();

    this->proxy.setType(QNetworkProxy::NoProxy);
    this->proxy.setHostName(QLatin1String(""));
    this->proxy.setUser(QLatin1String(""));
    this->proxy.setPassword(QLatin1String(""));
    this->proxy.setPort(80);

    QNetworkProxy::setApplicationProxy(this->proxy);

}

void
Proxy::randomProxy()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount() > 0) {
        int index = random(0,this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount()-1);
        qDebug() << "RANDOM PROXY INDEX " << index;
        qDebug() << "USING RANDOM PROXY " << this->ui->ProxyFetcherChecker_treeWidget->topLevelItem(index)->text(0);
        this->ui->ProxyFetcherChecker_treeWidget->setCurrentItem(this->ui->ProxyFetcherChecker_treeWidget->topLevelItem(index));
    }
}


void
Proxy::proxyHoppingbuttonClicked ( int id )
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    switch(id) {
        case    RandomProxyHopping::ProxyHoppingTimed:
        {
            this->ui->ProxyHopping_spinBox->setEnabled(true);
            this->ui->ProxyHopping_label->setEnabled(true);
            this->ui->ProxyHopping_pushButton->setEnabled(true);
            /* Disable other proxy controls so user can't mess with them while
             * random proxy hopping is active. Only leave "Get proxies"
             * button active ? */
            this->ui->ProxyFetcherChecker_treeWidget->setEnabled(false);

            this->ui->GetProxies_pushButton->setEnabled(false);
            this->ui->CheckProxies_pushButton->setEnabled(false);
            this->ui->ImportProxies_pushButton->setEnabled(false);
            this->ui->ExportProxies_pushButton->setEnabled(false);
        }
        break;
        case RandomProxyHopping::ProxyHoppingRequest:
        {
            /* Very first, no matter what, if timer is active stop it!!! */
            if(this->proxyHoppingTimer.isActive()) {
                this->ui->ProxyHopping_pushButton->click();
            }
            this->ui->ProxyHopping_spinBox->setEnabled(false);
            this->ui->ProxyHopping_label->setEnabled(false);
            this->ui->ProxyHopping_pushButton->setEnabled(false);

            /* Disable other proxy controls so user can't mess with them while
             * random proxy hopping is active. Only leave Get proxies
             * button active ? */
            this->ui->ProxyFetcherChecker_treeWidget->setEnabled(false);
            this->ui->GetProxies_pushButton->setEnabled(false);
            this->ui->CheckProxies_pushButton->setEnabled(false);
            this->ui->ImportProxies_pushButton->setEnabled(false);
            this->ui->ExportProxies_pushButton->setEnabled(false);

        }
        break;
        case RandomProxyHopping::NoProxyHopping:
        {
            /* Very first, no matter what, if timer is active stop it!!! */
            if(this->proxyHoppingTimer.isActive()) {
                this->ui->ProxyHopping_pushButton->click();
            }
            this->ui->ProxyHopping_spinBox->setEnabled(false);
            this->ui->ProxyHopping_label->setEnabled(false);
            this->ui->ProxyHopping_pushButton->setEnabled(false);
            this->ui->ProxyFetcherChecker_treeWidget->setEnabled(true);
            this->ui->GetProxies_pushButton->setEnabled(true);
            this->ui->CheckProxies_pushButton->setEnabled(true);
            this->ui->ImportProxies_pushButton->setEnabled(true);
            this->ui->ExportProxies_pushButton->setEnabled(true);
        }
        break;
    }

    /* Clear any previous proxy */
    this->ui->ClearProxy_pushButton->click();
    qDebug() << "ID clicked " << id;
}

/* =============================================
* Proxy parsers/fetchers start here
* =============================================
*/


/**************************************************/
/* Proxy parser for http://www.cool-proxy.net/proxies/http_proxy_list/sort:working_average/direction:desc/country_code:/port:/anonymous:1
 **************************************************/
void
Proxy::coolproxy(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity check */
    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    /* There were no problems with loading proxy site HTML page, continue with parsing */
    if(ok) {
        QWebElement doc = this->proxy_coolproxy->mainFrame()->documentElement();
        QWebElementCollection rows = doc.findAll(QLatin1String("tr"));
        QString text;
        bool    skip = false;

        /* Skip the first row that contains header and start scraping */
        register size_t n = rows.count();
        for(register unsigned int i = 1;i < n;++i) {
            QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));

            if(!cols.at(0).toPlainText().isEmpty()) {
                text = cols.at(0).toPlainText().trimmed() + QLatin1String(":") + cols.at(1).toPlainText().trimmed();

                /* Check if proxy is in blacklist before inserting ... */
                register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
                for(register unsigned int j = 0;j < m;++j) {
                    QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                    QString host = text.split(QLatin1String(":")).at(0);
                    if(item->text(0) == host) {
                        skip = true;
                        qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                        break;
                    }
                }

                /* Good, proxy was not found from blacklist. Put it into list */
                if(!skip) {
                    if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
                }
                skip = false;
            }
        }
    }

    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter); // Update progress

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    /* Proxy list ready? Good. Send signal that we are ready to check */
    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {

        /* If notification is enabled, notify user that proxy checking is about to start ... */
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }
        emit readyForProxyChecking();
    }
}



/**************************************************/
/* Proxy parser for http://www.samair.ru/proxy/proxy-01.htm
 **************************************************/

void
Proxy::samair(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity check */
    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    /* There were no problems with loading proxy site HTML page, continue with parsing */
    if(ok) {
        QWebElement doc = this->proxy_samair->mainFrame()->documentElement();
        QWebElement table = doc.findFirst(QLatin1String("table#proxylist"));
        QWebElementCollection rows = table.findAll(QLatin1String("tr"));
        QString text;
        bool    skip = false;
        /* Skip the first row that contains header and start scraping */
        register size_t n = rows.count();
        for(register unsigned int i = 1;i < n;++i) {
            QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
            if(!cols.at(0).toPlainText().isEmpty() && !cols.at(1).toPlainText().isEmpty() && cols.at(1).toPlainText() != QLatin1String("transparent")) {
                text = cols.at(0).toPlainText().trimmed();

                /* Check if proxy is in blacklist before inserting ... */
                register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
                for(register unsigned int j = 0;j < m;++j) {
                    QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                    QString host = text.split(QLatin1String(":")).at(0);
                    if(item->text(0) == host) {
                        skip = true;
                        qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                        break;
                    }
                }

                if(!skip) {
                    if(!Proxy::proxyList.contains(text))
                        Proxy::proxyList.insert(text);
                }
                skip = false;
            }
        }
    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);   // Update progress

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    /* Proxy list ready? Good. Send signal that we are ready to check */
    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {

        /* If notification is enabled, notify user that proxy checking is about to start ... */
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }

        emit readyForProxyChecking();
    }
}

/**************************************************/
/* Proxy parser for http://proxy-list.org/english/search.php?country=any&type=anonymous-and-elite&port=any&ssl=yes&p=1
 **************************************************/

void
Proxy::proxy_list(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {
    QWebElement doc = this->proxy_proxy_list->mainFrame()->documentElement();
    QWebElement table = doc.findFirst(QLatin1String("div#proxy-table"));
    QWebElementCollection rows = table.findAll(QLatin1String("li.proxy"));


    QString text;
    bool    skip = false;
    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        if(!rows.at(i).toPlainText().isEmpty()) {
            text = rows.at(i).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }
    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }
        emit readyForProxyChecking();
    }

}

void
Proxy::freeproxylist(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {
    QWebElement doc = this->proxy_freeproxylists->mainFrame()->documentElement();
    QWebElementCollection rows = doc.findAll(QLatin1String("tr.Even"));
    QString text;
    bool    skip = false;
    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(0).toPlainText().isEmpty()) {
            text = cols.at(0).toPlainText().trimmed() + QLatin1String(":") + cols.at(1).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }
    rows = doc.findAll(QLatin1String("tr.Odd"));
    register size_t n2 = rows.count();
    for(register unsigned int i = 0;i < n2;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(0).toPlainText().isEmpty()) {
            text = cols.at(0).toPlainText().trimmed() + QLatin1String(":") + cols.at(1).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }


    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }
        emit readyForProxyChecking();
    }

}

/* ========================================================= */
void
Proxy::xroxy(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {
    QWebElement doc = this->proxy_xroxy->mainFrame()->documentElement();
    QWebElementCollection rows = doc.findAll(QLatin1String("tr.row0"));
    QString text;
    bool    skip = false;

    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }
    rows = doc.findAll(QLatin1String("tr.row1"));
    register size_t n2 = rows.count();
    for(register unsigned int i = 0;i < n2;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }


    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }

        emit readyForProxyChecking();
    }
}


void
Proxy::xroxy1(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {
    QWebElement doc = this->proxy_xroxy1->mainFrame()->documentElement();
    QWebElementCollection rows = doc.findAll(QLatin1String("tr.row0"));
    QString text;
    bool    skip = false;

    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }
            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }
    rows = doc.findAll(QLatin1String("tr.row1"));
    register size_t n2 = rows.count();
    for(register unsigned int i = 0;i < n2;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }


    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }

        emit readyForProxyChecking();
    }
}


void
Proxy::xroxy2(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {
    QWebElement doc = this->proxy_xroxy2->mainFrame()->documentElement();
    QWebElementCollection rows = doc.findAll(QLatin1String("tr.row0"));
    QString text;
    bool    skip = false;

    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }
    rows = doc.findAll(QLatin1String("tr.row1"));
    register size_t n2 = rows.count();
    for(register unsigned int i = 0;i < n2;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));
        if(!cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(1).toPlainText().trimmed() + QLatin1String(":") + cols.at(2).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text))
                    Proxy::proxyList.insert(text);
            }
            skip = false;
        }
    }


    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;
    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }

        emit readyForProxyChecking();
    }
}

//http://free-proxy-list.net/
void
Proxy::free_proxy_list(bool ok)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    if(ok) {

    QWebElement doc = this->proxy_free_proxy_list->mainFrame()->documentElement();
    QWebElement table = doc.findFirst(QLatin1String("table"));
    QWebElementCollection rows = table.findAll(QLatin1String("tr"));
    QString text;
    bool    skip = false;
    /* Skip the first row that contains header and start scraping */
    register size_t n = rows.count();
    for(register unsigned int i = 1;i < n;++i) {
        QWebElementCollection   cols = rows.at(i).findAll(QLatin1String("td"));

        if(!cols.at(0).toPlainText().isEmpty() && !cols.at(1).toPlainText().isEmpty()) {
            text = cols.at(0).toPlainText().trimmed() + QLatin1String(":") + cols.at(1).toPlainText().trimmed();

            /* Check if proxy is in blacklist before inserting ... */
            register size_t m = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
            for(register unsigned int j = 0;j < m;++j) {
                QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(j);

                QString host = text.split(QLatin1String(":")).at(0);
                if(item->text(0) == host) {
                    skip = true;
                    qDebug() << "SKIPPED BLACKLISTED PROXY " << text;
                    break;
                }
            }

            if(!skip) {
                if(!Proxy::proxyList.contains(text)) {
                    Proxy::proxyList.insert(text);
                }
            }
            skip = false;
        }
    }

    }
    /* We start proxy checking when all fetchers have finished fetching (successfull or not) */
    ++Proxy::proxy_list_counter;

    this->ui->ProxyFetching_progressBar->setValue(Proxy::proxy_list_counter);

    qDebug() << "Proxy::proxy_list_counter " << Proxy::proxy_list_counter;

    if(Proxy::proxy_list_counter >= Proxy::NUMBER_OF_PROXY_SITES) {

        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchComplete_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Proxy Fetching Complete"),"Fetched "+ QString::number(Proxy::proxyList.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
        }

        emit readyForProxyChecking();
    }
}

/* ============================================= */
/* Here we got the data from worker.cpp module.
 * Each time this is called it means one proxy has been processed and
 * we can print the results here, in the main GUI thread */
void
Proxy::dataReady(const QStringList& data,const QColor& color)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "Ready data is: " << data;

    this->ui->ProxyChecking_progressBar->setValue(++Proxy::NUMBER_OF_PROXIES);

    /* Proxy data, in order, that will appear in this main GUI thread */
    /*
     * Host	Port	HTTP Status	Latency (in ms)		Country		SSL	Anonymous
    -------------------------------------------------------------------------------------------------------
    */

/* ============================================== */
    QString ip = data.at(0).trimmed();
    QString port = data.at(1).trimmed();
    QString httpStatus = data.at(2).trimmed();
    QString latency = data.at(3).trimmed();
    QString country = data.at(4).trimmed();
    QString ssl = data.at(5).trimmed();
    QString anonymous = data.at(6).trimmed();



    QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->ProxyFetcherChecker_treeWidget);
    item->setText(0,ip);
    item->setText(1,port);
    item->setText(2,httpStatus);
    item->setText(3,latency);
    item->setText(4,country);
    item->setText(5,ssl);
    item->setText(6,anonymous);

/* ============================================== */    
    register size_t columns = this->ui->ProxyFetcherChecker_treeWidget->columnCount();
    for(register unsigned int i = 0;i < columns;++i) {
        item->setTextColor(i,color);
    }

}

void
Proxy::proxyListReady()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->ProxyFetching_label->setVisible(false);
    this->ui->ProxyFetching_progressBar->setVisible(false);
    this->ui->ProxyFetching_progressBar->setValue(0);

    this->ui->ProxyFetcherChecker_treeWidget->clear();

    if(Proxy::proxyList.count() > 0) {
        QSet<QString>::const_iterator     i = Proxy::proxyList.constBegin();
        while (i != Proxy::proxyList.constEnd()) {
            qDebug() << *i;
            QStringList host = i->split(QLatin1String(":"),QString::SkipEmptyParts);

            QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->ProxyFetcherChecker_treeWidget,host);
            ++i;
        }

        this->proxies = Proxy::proxyList.toList();
    }

    /* If proxy list is empty then disable all random controls */
    if(this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount() > 0) {
        this->ui->ProxyHoppingTimed_radioButton->setEnabled(true);
        this->ui->ProxyHopping_spinBox->setEnabled(true);
        this->ui->ProxyHopping_label->setEnabled(true);
        this->ui->ProxyHopping_pushButton->setEnabled(true);
        this->ui->ProxyHoppingPageLoad_radioButton->setEnabled(true);
    } else {
        this->ui->ProxyHoppingTimed_radioButton->setEnabled(false);
        this->ui->ProxyHopping_spinBox->setEnabled(false);
        this->ui->ProxyHopping_label->setEnabled(false);
        this->ui->ProxyHopping_pushButton->setEnabled(false);
        this->ui->ProxyHoppingPageLoad_radioButton->setEnabled(false);
    }
}

void
Proxy::removeProxy()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QList<QTreeWidgetItem*> list = this->ui->ProxyFetcherChecker_treeWidget->selectedItems();
    foreach(QTreeWidgetItem* item,list) {
        delete item;
    }

    /* Clear the proxy that was accidentally set by selecting it from contextMenu */
    this->ui->ClearProxy_pushButton->click();
}

void Proxy::on_AddBlacklistedProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QList<QTreeWidgetItem*> list = this->ui->ProxyFetcherChecker_treeWidget->selectedItems();
    register size_t n = list.count();

    if(n > 0) {
        for(register unsigned int i = 0;i < n;++i) {
            QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->BlacklistedProxies_treeWidget);

            item->setText(0,list.at(i)->text(0));
            item->setFlags(item->flags() | Qt::ItemIsEditable);

            Proxy::proxyList.remove(list.at(i)->text(0) + QLatin1String(":") + list.at(i)->text(1));
            delete list.at(i);

        }

        this->ui->ProxyFetcherChecker_treeWidget->clearSelection();
        /* Clear the proxy that was accidentally set by selecting it from contextMenu */
        this->ui->ClearProxy_pushButton->click();

    } else {
        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->BlacklistedProxies_treeWidget);
        this->ui->BlacklistedProxies_treeWidget->scrollToBottom();
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        this->ui->BlacklistedProxies_treeWidget->editItem(item,0);
    }
}

void
Proxy::on_GetProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* Sanity check */
    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    this->ui->ClearProxy_pushButton->click();
    this->ui->removeNotWorkingProxies_pushButton->setEnabled(false);
    this->ui->ProxyFetching_label->setVisible(true);
    this->ui->ProxyFetching_progressBar->setVisible(true);
    this->ui->ProxyFetching_progressBar->setValue(0);

    QNetworkProxy::setApplicationProxy(this->proxy);

    Proxy::proxy_list_counter = 0;
    if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyFetchStart_checkBox->isChecked()) {
            Notify* notify = new Notify(QLatin1String("Proxy Fetching Started"),"",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
            notify->show();
    }

    this->proxy_freeproxylists->mainFrame()->load(QUrl(QLatin1String("http://www.freeproxylists.net/?pr=HTTPS&a[]=2&s=rs")));
    this->proxy_samair->mainFrame()->load(QUrl(QLatin1String("http://www.samair.ru/proxy/proxy-01.htm")));
    this->proxy_coolproxy->mainFrame()->load(QUrl(QLatin1String("http://www.cool-proxy.net/proxies/http_proxy_list/sort:working_average/direction:desc/country_code:/port:/anonymous:1")));
    this->proxy_free_proxy_list->mainFrame()->load(QUrl(QLatin1String("http://free-proxy-list.net/")));
    this->proxy_xroxy->mainFrame()->load(QUrl(QLatin1String("http://www.xroxy.com/proxylist.php?port=&type=Anonymous&ssl=ssl&country=&latency=&reliability=9000#table")));
    this->proxy_xroxy1->mainFrame()->load(QUrl(QLatin1String("http://www.xroxy.com/proxylist.php?port=&type=Anonymous&ssl=ssl&country=&latency=&reliability=9000&sort=reliability&desc=true&pnum=1#table")));
    this->proxy_xroxy2->mainFrame()->load(QUrl(QLatin1String("http://www.xroxy.com/proxylist.php?port=&type=Anonymous&ssl=ssl&country=&latency=&reliability=9000&sort=reliability&desc=true&pnum=2#table")));
    this->proxy_proxy_list->mainFrame()->load(QUrl(QLatin1String("http://proxy-list.org/english/search.php?country=any&type=anonymous-and-elite&port=any&ssl=yes&p=1")));

}


void
Proxy::on_ProxyFetcherChecker_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(current) {
        this->ui->Proxy_lineEdit->setText(current->text(0));
        this->ui->Proxy_spinBox->setValue(current->text(1).toUInt());
        this->ui->SetProxy_pushButton->click();
    }
}

void Proxy::on_SetProxy_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(!this->ui->Proxy_lineEdit->text().isEmpty()) {

        if(this->ui->ProxyTypeHTTP_radioButton->isChecked())
            this->proxy.setType(QNetworkProxy::HttpProxy);

        if(this->ui->ProxyTypeSOCKS5_radioButton->isChecked())
            this->proxy.setType(QNetworkProxy::Socks5Proxy);

        this->proxy.setHostName(this->ui->Proxy_lineEdit->text());
        this->proxy.setPort(this->ui->Proxy_spinBox->value());
        this->proxy.setUser(this->ui->ProxyUsername_lineEdit->text());
        this->proxy.setPassword(this->ui->ProxyPassword_lineEdit->text());

        QNetworkProxy::setApplicationProxy(this->proxy);

        QString text;

        if(this->proxyHoppingTimer.isActive())
            text += "Random proxy hopping timer started. Hopping to next random proxy in " + QString::number(this->ui->ProxyHopping_spinBox->value()) + " seconds ...\n";

        if(this->ui->ProxyHoppingPageLoad_radioButton->isChecked())
            text += "Random proxy hopping per page request active.\n";

        text += "Current Proxy: " + this->proxy.hostName() + ":" + QString::number(this->proxy.port());

        this->getMainWindow()->ui->currentProxy_label->setText(text);

    }
}

void
Proxy::on_ExportProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                                   QDir::currentPath(),
                                   tr("Any (*.*)"));

    if(!filename.isEmpty() && !filename.isNull()) {
        QFile   file(filename);
        if(file.open(QIODevice::WriteOnly)) {


            QList<QTreeWidgetItem*> list = this->ui->ProxyFetcherChecker_treeWidget->selectedItems();
            if(list.count() > 0 ) {
                register size_t n = list.count();
                QString  text;
                for(register unsigned int i = 0;i < n;++i) {
                    text = list.at(i)->text(0) + QLatin1String(":") + list.at(i)->text(1) + QLatin1String("\r\n");
                    file.write(text.toLatin1());
                }

                if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyExportComplete_checkBox->isChecked()) {
                        Notify* notify = new Notify(QLatin1String("Exported Proxies(s)"),"Exported "+ QString::number(list.count()) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                        notify->show();
                }
            } else {
                register unsigned int i = 0;

                register size_t n = this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount();
                for(;i < n;++i) {
                    QString tmp = this->ui->ProxyFetcherChecker_treeWidget->topLevelItem(i)->text(0) + QLatin1String(":") + this->ui->ProxyFetcherChecker_treeWidget->topLevelItem(i)->text(1) + QLatin1String("\r\n");
                    file.write(tmp.toLatin1());
                }

                if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyExportComplete_checkBox->isChecked()) {
                        Notify* notify = new Notify(QLatin1String("Exported Proxies(s)"),"Exported "+ QString::number(i) + " proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                        notify->show();
                }
            }
        }
    }
}

void Proxy::on_ImportProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    Q_ASSERT(this->mainWindow->notificationTab != NULL);

    int originalSize = 0;
    QString filename;

        filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                       QDir::currentPath(),
                                       tr("Any (*.*)"));

    if(!filename.isEmpty() && !filename.isNull()) {
        QFile   file(filename);
        if(file.open(QIODevice::ReadOnly)) {

            this->getMainWindow()->ui->currentProxy_label->setText(QLatin1String("Importing proxies. Please wait ... "));
            originalSize = Proxy::proxyList.count();

            int n = 0;
            bool    skip = false;
            while(!file.atEnd()) {
                QString line = file.readLine().trimmed();
                if(!line.isEmpty()) {
                    /* Check if proxy is in blacklist before inserting ... */

                    register size_t nn = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
                    for(register unsigned int i = 0;i < nn;++i) {
                        QTreeWidgetItem* item = this->ui->BlacklistedProxies_treeWidget->topLevelItem(i);
                        QString host = line.split(QLatin1String(":")).at(0);
                        /* Blacklisted proxy found don't insert it into Proxy::proxyList */
                        if(item->text(0) == host) {
                            skip = true;
                            qDebug() << "SKIPPED BLACKLISTED PROXY " << line;
                            break;
                        }
                    }

                    if(!skip) {
                        Proxy::proxyList.insert(line);
                        n++;
                    }
                    skip = false;
                }
            }
            file.close();

            qDebug() << n << " proxies importet";
            this->ui->ProxyFetcherChecker_treeWidget->clear();

            this->getMainWindow()->ui->currentProxy_label->setText(QLatin1String(""));

            if(Proxy::proxyList.size() > 0) {

                Proxy::total_number_of_proxies = Proxy::proxyList.count();
                if((Proxy::proxyList.count() - originalSize) > 0) {
                    if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyProxyImportComplete_checkBox->isChecked()) {
                        if(Proxy::proxyList.count() > originalSize) {
                            Notify* notify = new Notify(QLatin1String("Imported New Proxies(s)"),"Imported "+ QString::number((Proxy::proxyList.count() - originalSize)) + " new proxies(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                            notify->show();
                        }
                    }
                    this->getMainWindow()->ui->currentProxy_label->setText("Got " + QString("%1").arg(Proxy::proxyList.count() - originalSize) + " proxies.");
                }

                this->ui->ExportProxies_pushButton->setEnabled(true);
                this->ui->CheckProxies_pushButton->setEnabled(true);

                /* Enable random proxy hopping controls */
                if(this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount() > 0) {

                    this->ui->ProxyHoppingTimed_radioButton->setEnabled(true);
                    this->ui->ProxyHoppingPageLoad_radioButton->setEnabled(true);

                    /* Enable export proxy button */
                    this->ui->ExportProxies_pushButton->setEnabled(true);
                    this->ui->ImportProxies_pushButton->setEnabled(true);
                }


            }

        }
    }
    emit readyForProxyChecking();
}


void Proxy::on_CheckProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->removeNotWorkingProxies_pushButton->setEnabled(false);
    this->ui->ProxyChecking_label->setVisible(true);
    this->ui->ProxyChecking_progressBar->setVisible(true);
    this->ui->stopChecking_pushButton->setVisible(true);

    Proxy::NUMBER_OF_PROXIES = 0;

    this->ui->ProxyChecking_progressBar->setMaximum(this->proxies.count());
    this->ui->ProxyChecking_progressBar->setValue(0);
/* ===================================================================== */
    this->ui->ProxyFetcherChecker_treeWidget->clear();
    this->ui->ClearProxy_pushButton->click();

    worker = new Worker(proxies);
    worker->setProxyConnection(this->ui->proxyCheckConnection_lineEdit->text());
    worker->setProxySSL(this->ui->proxyCheckSSL_lineEdit->text());
    worker->setProxyJudge(this->ui->proxyJudge_lineEdit->text());
    workerThread = new QThread(this);
    worker->moveToThread(workerThread);

    connect(workerThread, SIGNAL(started()), worker, SLOT(doWork()));
    connect(worker,SIGNAL(finished()),workerThread,SLOT(quit()));
    connect(worker,SIGNAL(allProxiesFinished()),workerThread,SLOT(quit()));
    connect(worker,SIGNAL(finished()),worker,SLOT(deleteLater()));
    connect(workerThread,SIGNAL(finished()),workerThread,SLOT(deleteLater()));
    connect(this,SIGNAL(abortConnections()),worker,SLOT(slot_abortConnections()));
    connect(worker,SIGNAL(dataReady(QStringList,QColor)),this,SLOT(dataReady(QStringList,QColor)));

    workerThread->start(QThread::TimeCriticalPriority);
}

void
Proxy::on_stopChecking_pushButton_clicked()
{    
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    disconnect(worker,SIGNAL(dataReady(QStringList,QColor)),this,SLOT(dataReady(QStringList,QColor)));
    emit abortConnections();
    emit allProxiesFinished();
    workerThread->quit();
}

void
Proxy::slot_allProxiesFinished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->ProxyChecking_progressBar->setValue(this->ui->ProxyChecking_progressBar->maximum());
    this->ui->removeNotWorkingProxies_pushButton->setEnabled(true);

    qDebug() << "=========================================";
    qDebug() << "ALL " << Proxy::NUMBER_OF_PROXIES << "PROXIES FINISHED!";
    qDebug() << "NUMBER OF OFFLINE PROXIES " << Proxy::OFFLINE_PROXIES;
    qDebug() << "=========================================";

    this->ui->ProxyChecking_label->setVisible(false);
    this->ui->ProxyChecking_progressBar->setVisible(false);
    this->ui->stopChecking_pushButton->setVisible(false);
}

void
Proxy::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("proxy_settings"));
    if(settings.contains(QLatin1String("current_proxy_host"))) {

        this->ui->Proxy_lineEdit->setText(settings.value(QLatin1String("current_proxy_host")).toString());
        this->ui->Proxy_spinBox->setValue(settings.value(QLatin1String("current_proxy_port")).toInt());
    }

    if(settings.contains(QLatin1String("proxy_checker_connection")) && !settings.value(QLatin1String("proxy_checker_connection")).toString().isEmpty())
        this->ui->proxyCheckConnection_lineEdit->setText(settings.value(QLatin1String("proxy_checker_connection")).toString());
    this->ui->proxyCheckConnection_lineEdit->setCursorPosition(0);

    if(settings.contains(QLatin1String("proxy_checker_ssl")) && !settings.value(QLatin1String("proxy_checker_ssl")).toString().isEmpty())
        this->ui->proxyCheckSSL_lineEdit->setText(settings.value(QLatin1String("proxy_checker_ssl")).toString());
    this->ui->proxyCheckSSL_lineEdit->setCursorPosition(0);

    if(settings.contains(QLatin1String("default_proxy_judge")) && !settings.value(QLatin1String("default_proxy_judge")).toString().isEmpty())
        this->ui->proxyJudge_lineEdit->setText(settings.value(QLatin1String("default_proxy_judge")).toString());
    this->ui->proxyJudge_lineEdit->setCursorPosition(0);

    this->ui->ProxyHopping_spinBox->setValue(settings.value(QLatin1String("random_proxy_hopping_timeout")).toUInt());

    int size = settings.beginReadArray(QLatin1String("blacklisted_proxies"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->BlacklistedProxies_treeWidget);
        item->setText(0,settings.value(QLatin1String("proxy")).toString());
        item->setFlags(QFlag(settings.value(QLatin1String("flags")).toInt()));
    }
    settings.endArray();
    settings.endGroup();
}

void
Proxy::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("proxy_settings"));

    if(!this->ui->Proxy_lineEdit->text().isEmpty()) {

        settings.setValue(QLatin1String("current_proxy_host"),this->ui->Proxy_lineEdit->text());
        settings.setValue(QLatin1String("current_proxy_port"),this->ui->Proxy_spinBox->value());
    } else {
        settings.remove(QLatin1String("current_proxy_host"));
        settings.remove(QLatin1String("current_proxy_port"));
    }

    settings.setValue(QLatin1String("proxy_checker_connection"),this->ui->proxyCheckConnection_lineEdit->text());
    settings.setValue(QLatin1String("proxy_checker_ssl"),this->ui->proxyCheckSSL_lineEdit->text());
    settings.setValue(QLatin1String("default_proxy_judge"),this->ui->proxyJudge_lineEdit->text());
    settings.setValue(QLatin1String("random_proxy_hopping_timeout"),this->ui->ProxyHopping_spinBox->value());

    settings.remove(QLatin1String("blacklisted_proxies"));  /* remove old cruft out first ... */
    settings.beginWriteArray(QLatin1String("blacklisted_proxies"));

    int size = this->ui->BlacklistedProxies_treeWidget->topLevelItemCount();
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("proxy"),this->ui->BlacklistedProxies_treeWidget->topLevelItem(i)->text(0));
        settings.setValue(QLatin1String("flags"),(int)this->ui->BlacklistedProxies_treeWidget->topLevelItem(i)->flags());
    }
    settings.endArray();
    settings.endGroup();
}

void Proxy::on_RemoveBlacklistedProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QList<QTreeWidgetItem*> list = this->ui->BlacklistedProxies_treeWidget->selectedItems();

    if(list.count() > 0 ) {
        register size_t  n = list.count();
        for(register unsigned int i = 0;i < n;++i)     {
            delete list.at(i);
        }
    }
}

void Proxy::on_ProxyHopping_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(!this->proxyHoppingTimer.isActive()) {
        this->proxyHoppingTimer.start(this->ui->ProxyHopping_spinBox->value() * 1000);
        this->ui->ProxyHopping_pushButton->setText(QLatin1String("Stop"));
        this->randomProxy();
        qDebug() << "TIMER STARTED";
    } else {
        this->proxyHoppingTimer.stop();
        this->ui->ProxyHopping_pushButton->setText(QLatin1String("Start"));
        this->ui->ClearProxy_pushButton->click();
        this->getMainWindow()->ui->currentProxy_label->setText(QLatin1String("Random proxy hopping timer stopped."));
        qDebug() << "TIMER STOPPED";
    }
}


/* Just an quick, convenient helper function to remove not working proxies */
void Proxy::on_removeNotWorkingProxies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    register size_t size = this->ui->ProxyFetcherChecker_treeWidget->topLevelItemCount();
    for(register unsigned int i = 0;i < size;++i) {
        QTreeWidgetItem*    item = this->ui->ProxyFetcherChecker_treeWidget->topLevelItem(i);

        if(!item->text(2).startsWith(QLatin1String("200 ")) || !item->text(5).startsWith(QLatin1String("Yes"))) {
            item->setSelected(true);
        }
         if(item->text(6).startsWith(QLatin1String("Anonymous")) || item->text(6).startsWith(QLatin1String("High-anonymous"))) {

         } else {
             item->setSelected(true);
         }
    }

    QList<QTreeWidgetItem*> list = this->ui->ProxyFetcherChecker_treeWidget->selectedItems();
    foreach(QTreeWidgetItem* item,list) {
        delete item;
    }
}
