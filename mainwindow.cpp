/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "dialog.h"
#include "browser.h"

#include "general.h"
#include "ui_general.h"

#include "trackerblocker.h"
#include "ui_trackerblocker.h"

#include "cookie.h"
#include "ui_cookie.h"

#include "proxy.h"
#include "ui_proxy.h"

#include "encryption.h"
#include "ui_encryption.h"

#include "cache.h"
#include "ui_cache.h"

#include "downloads.h"
#include "ui_downloads.h"

#include "notification.h"
#include "ui_notification.h"

#include "log.h"
#include "ui_log.h"

#include "plugins.h"
#include "ui_plugins.h"

#include "serverinfo.h"
#include "ui_serverinfo.h"

#include "dialog.h"

#include <QUrl>
#include <QTreeWidget>

#include <QDebug>
#include <QAction>
#include <QWhatsThis>
#include <QSettings>
#include <QDesktopServices>
#include <QFileDialog>

#include <QMessageBox>
#include <QtGlobal>
#include <QtWebKit>

#include "mytreewidget.h"
#include "colors.h"

#include <QPrinter>
#include <QPrintPreviewDialog>
#include <QTextEdit>

#include <QWebView>


/* struct to keep static info about possible ciphers.
 * Ideally, this should not be needed and everything should be possible to query dynamically
 * from Qt which in turn queries this stuff from OpenSSL. However, see pfs below */

struct cipherList {
    QString             name;
    QString             protocol;
    bool                pfs;        // Is there a way to query cipher PFS from OpenSSL ???
                                    // If there is then this structure could be made obsolete (Yay!!!).
    bool                enabled;
};

/* Needed for "Settings" tabwidget tab order/focus saving */
QList<QWidget*> widgets;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),settingsFile("config.ini"),findOptions(0),
    ui(new Ui::MainWindow)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    ui->setupUi(this);

    /* Ugly hack to make downnload tab upper section bigger.
     * Need Qt GUI layout expert to fix this */
    QList<int>  sizes;
    sizes << 3000 << 10;
    this->ui->splitter_3->setSizes(sizes);

    this->setWindowTitle(QString("CyberDragon - ") + CYBERDRAGON_VERSION);

    /* ======================================= */
    /* Set "What's This" button and menu entry */
    /* ======================================= */
    QAction*    whatsthis = QWhatsThis::createAction(this);

    this->ui->mainToolBar->addSeparator();
    this->ui->mainToolBar->insertWidget(this->ui->actionHide_Show_Settings,new QLabel(QLatin1String(" Find text "),this));
    this->findBox = new QLineEdit(this);

    connect(this->findBox,SIGNAL(textChanged(QString)),this,SLOT(slot_FindText(QString)));
    this->ui->mainToolBar->insertWidget(this->ui->actionHide_Show_Settings,this->findBox);
    this->ui->mainToolBar->insertAction(this->ui->actionHide_Show_Settings,this->ui->actionFindForward);
    this->ui->mainToolBar->insertAction(this->ui->actionHide_Show_Settings,this->ui->actionFindBackward);

    this->findHighlightAll = new QCheckBox(QLatin1String("Highlight all"),this);
    this->findCaseSensitive = new QCheckBox(QLatin1String("Case sensitive"),this);

    connect(this->findHighlightAll,SIGNAL(clicked()),this,SLOT(slot_FindText()));
    connect(this->findCaseSensitive,SIGNAL(clicked()),this,SLOT(slot_FindText()));

    this->ui->mainToolBar->insertWidget(this->ui->actionHide_Show_Settings,this->findHighlightAll);
    this->ui->mainToolBar->insertWidget(this->ui->actionHide_Show_Settings,this->findCaseSensitive);
    this->ui->mainToolBar->addSeparator();
    this->ui->mainToolBar->insertAction(this->ui->actionHide_Show_Settings,whatsthis);

    /* Load the modules to each tab in Settings, in this order,
     * and then read the actual *wanted* order (because it's user changeable)
     * from config.ini later and rearrange */


    this->generalTab = new general(this);
    this->ui->General_scrollArea->setWidget(this->generalTab);
    this->ui->General_scrollArea->setSizePolicy(this->generalTab->sizePolicy());

    this->trackerblockerTab = new trackerblocker(this);    
    this->ui->TrackerBlocker_scrollArea->setWidget(this->trackerblockerTab);
    this->ui->TrackerBlocker_scrollArea->setSizePolicy(this->trackerblockerTab->sizePolicy());

    this->cookieTab = new Cookie(this);
    this->ui->CookieControl_scrollArea->setWidget(this->cookieTab);
    this->ui->CookieControl_scrollArea->setSizePolicy(this->cookieTab->sizePolicy());

    this->proxyTab = new Proxy(this);
    this->ui->Proxy_scrollArea->setWidget(this->proxyTab);
    this->ui->Proxy_scrollArea->setSizePolicy(this->proxyTab->sizePolicy());

    this->encryptionTab = new encryption(this);
    this->ui->Encryption_scrollArea->setWidget(this->encryptionTab);
    this->ui->Encryption_scrollArea->setSizePolicy(this->encryptionTab->sizePolicy());

    this->loadCiphers();

    this->downloadsTab = new downloads(this);
    this->ui->Downloads_scrollArea->setWidget(this->downloadsTab);
    this->ui->Downloads_scrollArea->setSizePolicy(this->downloadsTab->sizePolicy());

    this->notificationTab = new notification(this);
    this->ui->Notification_scrollArea->setWidget(this->notificationTab);
    this->ui->Notification_scrollArea->setSizePolicy(this->notificationTab->sizePolicy());

    this->logTab = new Log(this);
    this->ui->Log_scrollArea->setWidget(this->logTab);
    this->ui->Log_scrollArea->setSizePolicy(this->logTab->sizePolicy());

    this->pluginTab = new Plugins(this);
    this->ui->Plugins_scrollArea->setWidget(this->pluginTab);
    this->ui->Plugins_scrollArea->setSizePolicy(this->pluginTab->sizePolicy());

    /* ========================= */
    /* Set up Downloads delegate */
    /* ========================= */
        TorrentViewDelegate*    itemDelegate = new TorrentViewDelegate(this);


        this->downloadsTab->ui->torrentView->setItemDelegate(itemDelegate);
        this->downloadsTab->ui->torrentView->setSelectionBehavior(QAbstractItemView::SelectRows);
        this->downloadsTab->ui->torrentView->setAlternatingRowColors(true);
        this->downloadsTab->ui->torrentView->setRootIsDecorated(false);
        this->downloadsTab->ui->torrentView->setColumnWidth(Download::StartStopButton_Col,16+10);
        this->downloadsTab->ui->torrentView->header()->setSectionResizeMode(Download::StartStopButton_Col,QHeaderView::Fixed);
        this->downloadsTab->ui->torrentView->header()->resizeSection(Download::StartStopButton_Col,16+10);
        this->downloadsTab->ui->torrentView->header()->setSectionResizeMode(Download::Progress_Col,QHeaderView::Fixed);
        this->downloadsTab->ui->torrentView->header()->resizeSection(Download::Progress_Col,100);

        connect(this->downloadsTab->ui->torrentView,SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),this,SLOT(openFile(QTreeWidgetItem*,int)));
        connect(itemDelegate,SIGNAL(buttonClicked(QModelIndex)),this,SLOT(startstopDownload(QModelIndex)));

    connect(this->ui->Browser_tabWidget,SIGNAL(tabCloseRequested(int)),this,SLOT(closeTab(int)));
    connect(this->ui->Browser_tabWidget,SIGNAL(currentChanged(int)),this,SLOT(tabChanged(int)));
    connect(this->ui->Browser_tabWidget->tabBar(),SIGNAL(tabMoved(int,int)),this,SLOT(tabMoved(int,int)));

    this->cacheTab = new Cache(this);
    this->cacheTab->setMainWindow(this);
    this->ui->Cache_scrollArea->setWidget(this->cacheTab);
    this->ui->Cache_scrollArea->setSizePolicy(this->cacheTab->sizePolicy());

    /* Settings tabwidget has now been populated & initialized with each corresponding module
     * (that is, each module reads it's own settings from config.ini and acts accordingly).
     * Now read the rest of the settings for MainWindow too from config.ini */
    this->loadSettings();

    /* Create the very first browser tab and load it with home page */
    this->addTab(this->generalTab->ui->HomePage_lineEdit->text());

/* =========================*/
/* Cache is now ready. "Click" the cache button so that we can see the current status */
/* =========================*/
    this->cacheTab->update();


}

MainWindow::~MainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->saveSettings();
    delete ui;
}

Browser*
MainWindow::addTab(const QString& url)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    Browser*    browser = new Browser(this,url);

    connect(browser,SIGNAL(linkHovered(QString,QString,QString)),this,SLOT(linkHovered(QString,QString,QString)));
    connect(browser,SIGNAL(titleChanged(QString)),this,SLOT(titleChanged(QString)));
    connect(browser,SIGNAL(unsupportedContent(QNetworkReply*)),this,SLOT(slot_unsupportedContent(QNetworkReply*)));

    Q_ASSERT(this->logTab != NULL);
    connect(browser,SIGNAL(dataReady(QStringList,QColor)),this->logTab,SLOT(dataReady(QStringList,QColor)));

    connect(browser->getWebPage(),SIGNAL(downloadRequested(QNetworkRequest)),this,SLOT(slot_downloadRequested(QNetworkRequest)));
    connect(browser,SIGNAL(activateMixedContentBlocking(bool)),browser->getNetworkAccessManager(),SLOT(activateMixedContentBlocking(bool)));


    /* ====================================================================
     * Tracker Blocker
     * ==================================================================== */

    Q_ASSERT(this->trackerblockerTab != NULL);

    MyTreeWidget*    trackersBlocked = new MyTreeWidget;

        /*Tracker Blocker List. Set to fixed height of 200 px for now.
         * Need Qt GUI layout expert here. */
        trackersBlocked->setSortingEnabled(true);
        trackersBlocked->header()->setDefaultSectionSize(200);
        trackersBlocked->header()->setMinimumSectionSize(100);
        trackersBlocked->setMinimumHeight(200);
        {
            QStringList	headers;

            headers << "URL" << "Blocked by rule";
            trackersBlocked->setHeaderLabels(headers);
        }

        this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->addWidget(trackersBlocked);
        MyNetworkAccessManager* manager = browser->getNetworkAccessManager();

        connect(manager,SIGNAL(trackerBlocked(QStringList)),trackersBlocked,SLOT(updateTrackersBlocked(QStringList)));
        connect(browser,SIGNAL(clearTrackerList()),trackersBlocked,SLOT(clear()));        

    /* ================================================================
     * CookieList
     * ================================================================ */


    Q_ASSERT(this->cookieTab != NULL);

        MyTreeWidget *cookieList = new MyTreeWidget;

        cookieList->setSortingEnabled(true);
        cookieList->header()->setProperty("showSortIndicator", QVariant(true));
        cookieList->setMinimumHeight(200);
        {
            /* Time	Action	Domain	Path	Name=Value	Expiration time		Secure		HttpOnly
            /* --------------------------------------------------------------------------------------------------- */
            QStringList	headers;

            headers << "Time" << "Action" << "Domain" << "Path" << "Name=Value" << "Expiration time" << "Secure" << "HttpOnly";
            cookieList->setHeaderLabels(headers);
        }

        cookieList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        cookieList->header()->setSortIndicator(0,Qt::AscendingOrder);        

        connect(browser,SIGNAL(clearCookieList()),cookieList,SLOT(clear()));


        MyTreeWidget *AllowdCookieList = new MyTreeWidget;

        AllowdCookieList->setSortingEnabled(true);
        AllowdCookieList->header()->setProperty("showSortIndicator", QVariant(true));
        AllowdCookieList->setMinimumHeight(200);

        {
            QStringList	headers;

            headers << "Domain" << "Path"  << "Name=Value";
            AllowdCookieList->setHeaderLabels(headers);
        }

        AllowdCookieList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        AllowdCookieList->header()->setSortIndicator(0,Qt::AscendingOrder);

        MyCookieJar*    cookieJar = browser->getCookieJar();
        connect(cookieJar,SIGNAL(cookieUpdated(QStringList)),cookieList,SLOT(updateCookieList(QStringList)));

        connect(cookieJar,SIGNAL(updateAllCookiesList(QList<QNetworkCookie>)),AllowdCookieList,SLOT(updateAllowedCookieList(QList<QNetworkCookie>)));
        connect(this->cookieTab,SIGNAL(ClearAllCookies()),cookieJar,SLOT(deleteAllCookies()));


        this->cookieTab->ui->CookieList_stackedWidget->addWidget(cookieList);
        this->cookieTab->ui->AllowedCookies_stackedWidget->addWidget(AllowdCookieList);

    /* =================================================================
     * Encryption
     * ================================================================= */
    /* MCB .... */


    Q_ASSERT(this->encryptionTab != NULL);
        MyTreeWidget*   blockedMCBURLs = new MyTreeWidget;

        blockedMCBURLs->setSortingEnabled(true);
        blockedMCBURLs->header()->setProperty("showSortIndicator", QVariant(true));

        {
            QStringList	headers;
            headers << "Blocked Content URLs";
            blockedMCBURLs->setHeaderLabels(headers);
        }

        blockedMCBURLs->setSelectionMode(QAbstractItemView::ExtendedSelection);

        connect(browser,SIGNAL(clearMixedContentBlockingList()),blockedMCBURLs,SLOT(clear()));
        connect(browser->getNetworkAccessManager(),SIGNAL(halfEncryptedURLFound(QString)),blockedMCBURLs,SLOT(halfEncryptedSlotFound(QString)));

        this->encryptionTab->ui->BlockedMCB_stackedWidget->addWidget(blockedMCBURLs);


        QTextEdit*  cipherDetails_textEdit = new QTextEdit(this->ui->cipherDetails_stackedWidget);
        connect(browser,SIGNAL(cipherDetailsChanged(QString)),cipherDetails_textEdit,SLOT(setHtml(QString)));

        QFont font;
        font.setKerning(false);
        font.setStyleStrategy(QFont::PreferAntialias);
        font.setFamily(QLatin1String("Courier"));
        font.setStyleHint(QFont::Monospace);
        font.setFixedPitch(true);
        font.setPointSize(10);


        cipherDetails_textEdit->setFont(font);
        cipherDetails_textEdit->setWordWrapMode(QTextOption::NoWrap);
        cipherDetails_textEdit->setAcceptRichText(true);

        this->ui->cipherDetails_stackedWidget->addWidget(cipherDetails_textEdit);

        /* =================================================================
         * Server Info
         * ================================================================= */
        ServerInfo* serverInfo = browser->getServerInfo();
        connect(this->ui->serverInfoLookup_checkBox,SIGNAL(toggled(bool)),serverInfo,SLOT(setEnabled(bool)));
        connect(this->ui->serverInfoLookup_checkBox,SIGNAL(toggled(bool)),this->ui->googleMapsSettings_groupBox,SLOT(setEnabled(bool)));

        this->ui->ServerInfo_stackedWidget->addWidget(serverInfo);

        // Instance of browser is now ready. Add it to tab page
        int index = this->ui->Browser_tabWidget->addTab(browser,QLatin1String(""));
        this->ui->Browser_tabWidget->setCurrentIndex(index);
        this->tabChanged(index);

        return  browser;
}


void
MainWindow::linkHovered(const QString& link,const QString& title, const QString& textContent)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->statusBar()->showMessage(link);
}

void
MainWindow::loadStarted()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}


void
MainWindow::titleChanged(const QString& title)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QWidget*    source_browser = qobject_cast<QWidget*>(sender());

    int index = this->ui->Browser_tabWidget->indexOf(source_browser);
    QString text = title;  /* Take a copy of the title */

    if(title.isEmpty()) {   // There was no title! This happens, for example, when loading image to tab page
                            // We make our own title then from the URL!

        Browser*    browser = qobject_cast<Browser*>(sender());
        if(browser)         {

            text = browser->getUrl().toDisplayString();
            this->ui->Browser_tabWidget->setTabToolTip(index,text);

            /* Set tab title to fixed 16 chars */
            if(text.length() > 16)
                text.truncate(16);
            else
                text.leftJustified(16,' ');

            this->ui->Browser_tabWidget->setTabText(index,text);

        }
    } else {

        /* Set tab title to fixed 16 chars */
        if(text.length() > 16)
            text.truncate(16);
        else
            text.leftJustified(16,' ');

        this->ui->Browser_tabWidget->setTabText(index,text);
        this->ui->Browser_tabWidget->setTabToolTip(index,title);

    }
}

void
MainWindow::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("general_settings"));

    this->zoomFactor = settings.value(QLatin1String("zoom_factor")).toReal();

    this->ui->actionHide_Show_Settings->setChecked(settings.value(QLatin1String("hide_settings")).toBool());
    this->ui->actionHide_Show_Settings->triggered(settings.value(QLatin1String("hide_settings")).toBool());

    settings.endGroup();
    settings.beginGroup(QLatin1String("tabs"));

    /* Save widgets & clear */
    for(int i = 0;i < this->ui->Settings_tabWidget->count();++i) {
        this->ui->Settings_tabWidget->widget(i)->setObjectName(this->ui->Settings_tabWidget->tabText(i));
        widgets.append(this->ui->Settings_tabWidget->widget(i));
    }

    int size = settings.beginReadArray(QLatin1String("tab"));
    if(size > 0)
        this->ui->Settings_tabWidget->clear();


    /* Now repopulate tabbar according to new order */

    for (int row = 0; row < size; ++row) {
        settings.setArrayIndex(row);

        for(int col = 0;col < widgets.count();++col) {
            if(widgets.at(col)->objectName() == settings.value(QLatin1String("name")).toString()) {
                this->ui->Settings_tabWidget->addTab(widgets.at(col),widgets.at(col)->objectName());
            }
        }
    }

    if(widgets.count() > size) {
        for(int i = 0;i < widgets.count();++i) {
            if(widgets.at(i)->objectName() == this->ui->Settings_tabWidget->widget(i)->objectName()) {
                widgets.removeAt(i);
            }
        }
        /* Now the widget has left only those tabs that did not appear in config.ini.
         * Append them */
        for(int i = 0;i < widgets.count();++i) {
            this->ui->Settings_tabWidget->addTab(widgets.at(i),widgets.at(i)->objectName());
        }
    }


    settings.endArray();
    this->ui->Settings_tabWidget->setCurrentIndex(settings.value(QLatin1String("current")).toUInt());
    settings.endGroup();

/**************************************************/
    settings.beginGroup(QLatin1String("server_info"));

    this->ui->zoom_spinBox->setValue(settings.value(QLatin1String("zoom")).toUInt());
    this->ui->width_spinBox->setValue(settings.value(QLatin1String("width")).toUInt());
    this->ui->height_spinBox->setValue(settings.value(QLatin1String("height")).toUInt());
    this->ui->scale_spinBox->setValue(settings.value(QLatin1String("scale")).toUInt());
    this->ui->mapType_comboBox->setCurrentIndex(settings.value(QLatin1String("type")).toUInt());

    this->ui->googleMapsLookup_checkBox->setChecked(settings.value(QLatin1String("enable_google_maps_lookup")).toBool());
    this->ui->googleMapsLookup_checkBox->toggled(settings.value(QLatin1String("enable_google_maps_lookup")).toBool());

    this->ui->serverInfoLookup_checkBox->setChecked(settings.value(QLatin1String("enable_server_info_lookup")).toBool());
    this->ui->serverInfoLookup_checkBox->toggled(settings.value(QLatin1String("enable_server_info_lookup")).toBool());

    settings.endGroup();

}

void
MainWindow::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("general_settings"));

    settings.setValue(QLatin1String("zoom_factor"),this->zoomFactor);
    settings.setValue(QLatin1String("hide_settings"),this->ui->actionHide_Show_Settings->isChecked());

    settings.endGroup();


    settings.beginGroup(QLatin1String("tabs"));
    settings.setValue(QLatin1String("current"),this->ui->Settings_tabWidget->currentIndex());
    settings.beginWriteArray(QLatin1String("tab"));
    for(int i = 0;i < this->ui->Settings_tabWidget->count();++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("name"),this->ui->Settings_tabWidget->tabText(i));
    }
    settings.endArray();
    settings.endGroup();


    settings.beginGroup(QLatin1String("server_info"));

    settings.setValue(QLatin1String("enable_server_info_lookup"),this->ui->serverInfoLookup_checkBox->isChecked());
    settings.setValue(QLatin1String("enable_google_maps_lookup"),this->ui->googleMapsLookup_checkBox->isChecked());

    settings.setValue(QLatin1String("zoom"),this->ui->zoom_spinBox->value());
    settings.setValue(QLatin1String("width"),this->ui->width_spinBox->value());
    settings.setValue(QLatin1String("height"),this->ui->height_spinBox->value());
    settings.setValue(QLatin1String("scale"),this->ui->scale_spinBox->value());

    qDebug() << "SERVER INFO CURRENT INDEX " << this->ui->mapType_comboBox->currentIndex();

    settings.setValue(QLatin1String("type"),this->ui->mapType_comboBox->currentIndex());
    settings.endGroup();
}




void
MainWindow::closeTab(int index)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(this->ui->Browser_tabWidget->currentIndex() == index) {

        Q_ASSERT(this->trackerblockerTab != NULL);
        QWidget*    trackerTreeWidget = this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->widget(index);
            if(trackerTreeWidget) {
                this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->removeWidget(trackerTreeWidget);
                trackerTreeWidget->deleteLater();
            }

        /* Time	Action	Domain	Path	Name=Value	Expiration time		Secure		HttpOnly
        /* --------------------------------------------------------------------------------------------------- */

        Q_ASSERT(this->cookieTab != NULL);
            QWidget*    cookieListTreeWidget = this->cookieTab->ui->CookieList_stackedWidget->widget(index);
            if(cookieListTreeWidget) {
                this->cookieTab->ui->CookieList_stackedWidget->removeWidget(cookieListTreeWidget);
                cookieListTreeWidget->deleteLater();
            }

            QWidget*    AllowedCookieListTreeWidget = this->cookieTab->ui->AllowedCookies_stackedWidget->widget(index);
            if(AllowedCookieListTreeWidget) {
                this->cookieTab->ui->AllowedCookies_stackedWidget->removeWidget(AllowedCookieListTreeWidget);
                AllowedCookieListTreeWidget->deleteLater();
            }

        // Blocked Content URLs
        // ---------------------
        Q_ASSERT(this->encryptionTab != NULL);

            QWidget*    blockedMixedContentTreeWidget = this->encryptionTab->ui->BlockedMCB_stackedWidget->widget(index);
            if(blockedMixedContentTreeWidget) {
                this->encryptionTab->ui->BlockedMCB_stackedWidget->removeWidget(blockedMixedContentTreeWidget);
                blockedMixedContentTreeWidget->deleteLater();
            }

            QWidget*    cipherDetails_label = this->ui->cipherDetails_stackedWidget->widget(index);
            if(cipherDetails_label) {
                this->ui->cipherDetails_stackedWidget->removeWidget(cipherDetails_label);
                cipherDetails_label->deleteLater();
            }

            QWidget*    serverInfo = this->ui->ServerInfo_stackedWidget->widget(index);
            if(serverInfo) {
                this->ui->ServerInfo_stackedWidget->removeWidget(serverInfo);
                serverInfo->deleteLater();
            }



        QWidget*    myBrowserWidget = this->ui->Browser_tabWidget->widget(index);
        if(myBrowserWidget) {
            myBrowserWidget->deleteLater();
        }

        this->ui->Browser_tabWidget->removeTab(index);
    }
}

void
MainWindow::tabChanged(int n)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "TAB CHANGED " << n << " !!!!!!!!!!!!!!!!!!";

        Q_ASSERT(this->trackerblockerTab != NULL);
        this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->setCurrentIndex(n);

        Q_ASSERT(this->cookieTab != NULL);
        this->cookieTab->ui->CookieList_stackedWidget->setCurrentIndex(n);
        this->cookieTab->ui->AllowedCookies_stackedWidget->setCurrentIndex(n);

        Q_ASSERT(this->encryptionTab != NULL);
        this->encryptionTab->ui->BlockedMCB_stackedWidget->setCurrentIndex(n);
        this->ui->cipherDetails_stackedWidget->setCurrentIndex(n);

        this->ui->ServerInfo_stackedWidget->setCurrentIndex(n);
}

Browser*
MainWindow::currentTab()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  qobject_cast<Browser*>(this->ui->Browser_tabWidget->currentWidget());
}


void
MainWindow::tabMoved(int fromIndex,int toIndex)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

        Q_ASSERT(this->trackerblockerTab != NULL);
        QWidget*    trackerBlocker_treeWidget = this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->widget(fromIndex);
        this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->removeWidget(trackerBlocker_treeWidget);
        this->trackerblockerTab->ui->TrackerBlocker_stackedWidget->insertWidget(toIndex,trackerBlocker_treeWidget);

        Q_ASSERT(this->cookieTab != NULL);
        QWidget*    cookieList_treeWidget = this->cookieTab->ui->CookieList_stackedWidget->widget(fromIndex);
        this->cookieTab->ui->CookieList_stackedWidget->removeWidget(cookieList_treeWidget);
        this->cookieTab->ui->CookieList_stackedWidget->insertWidget(toIndex,cookieList_treeWidget);

        QWidget*    AllowedCookieList_treeWidget = this->cookieTab->ui->AllowedCookies_stackedWidget->widget(fromIndex);
        this->cookieTab->ui->AllowedCookies_stackedWidget->removeWidget(AllowedCookieList_treeWidget);
        this->cookieTab->ui->AllowedCookies_stackedWidget->insertWidget(toIndex,AllowedCookieList_treeWidget);

        Q_ASSERT(this->encryptionTab != NULL);
        QWidget*    blockedMixedContent_treeWidget = this->encryptionTab->ui->BlockedMCB_stackedWidget->widget(fromIndex);
        this->encryptionTab->ui->BlockedMCB_stackedWidget->removeWidget(blockedMixedContent_treeWidget);
        this->encryptionTab->ui->BlockedMCB_stackedWidget->insertWidget(toIndex,blockedMixedContent_treeWidget);


        QWidget*    cipherDetails_label = this->ui->cipherDetails_stackedWidget->widget(fromIndex);
        this->ui->cipherDetails_stackedWidget->removeWidget(cipherDetails_label);
        this->ui->cipherDetails_stackedWidget->insertWidget(toIndex,cipherDetails_label);


        QWidget*    serverInfo = this->ui->ServerInfo_stackedWidget->widget(fromIndex);
        this->ui->ServerInfo_stackedWidget->removeWidget(serverInfo);
        this->ui->ServerInfo_stackedWidget->insertWidget(toIndex,serverInfo);
}

/****************************************************/
/* Download handling */
/***************************************************/
void
MainWindow::openFile(QTreeWidgetItem * item, int column)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->downloadsTab != NULL);
    if(item) {
        Job job = jobs.at(this->downloadsTab->ui->torrentView->indexOfTopLevelItem(item));

        if(job.client->isFinished() && job.client->isStopped()) {
            qDebug() << "CLIENT destination Dir " << job.destinationDirectory;
            QDesktopServices::openUrl(QUrl(QLatin1String("file:///") + job.destinationDirectory));
        }                        \
    }
}

const TorrentClient*
MainWindow::clientForRow(int row) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    // Return the client at the given row.
    return jobs.at(row).client;
}

void
MainWindow::startstopDownload(const QModelIndex &index)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->downloadsTab != NULL);
    if(index.isValid()) {
        bool stopped = jobs.at(index.row()).client->isStopped();
        bool finished = jobs.at(index.row()).client->isFinished();

        if(!stopped) {
            qDebug() << "STOPPING DOWNLOAD AT INDEX " << index.row();

            jobs.at(index.row()).client->stop();

            QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(index.row());

            if (item) {
                item->setText(1, QString::number(0));
            }
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setText(Download::Status_Col,QLatin1String("Cancelled"));
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setText(Download::FileSize_Col, 0);
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setToolTip(Download::StartStopButton_Col,QLatin1String("Start download"));
            } else {
                /* It is stopped, restarting it all again */
                jobs.at(index.row()).client->start();
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setText(Download::Progress_Col,QLatin1String("0"));
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setText(Download::Status_Col,QLatin1String("Starting..."));
                this->downloadsTab->ui->torrentView->topLevelItem(index.row())->setToolTip(Download::StartStopButton_Col,QLatin1String("Cancel download"));
            }
    }
}

void
MainWindow::failedProgress(QNetworkReply* reply)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);

    Q_ASSERT(this->downloadsTab != NULL);
    QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(row);

    if (item) {
        item->setText(Download::Progress_Col, QString::number(0));
        item->setText(Download::Status_Col,"Error! " + reply->errorString());

        Q_ASSERT(this->notificationTab != NULL);
        if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadError_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Error!"),reply->errorString(),this->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
                notify->show();
        }
        this->downloadsTab->ui->torrentView->resizeColumnToContents(Download::Status_Col);
        item->setToolTip(Download::Status_Col,reply->errorString());

        jobs.at(row).client->stop();
        QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(row);
        if (item) {
            item->setText(1, QString::number(0));
        }
        this->downloadsTab->ui->torrentView->topLevelItem(row)->setToolTip(Download::StartStopButton_Col,QLatin1String("Start download"));
    }

}

void
MainWindow::finishedProgress(QNetworkReply*)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = rowOfClient(client);


    Q_ASSERT(this->downloadsTab != NULL);
    QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(row);
    if (item) {
        QModelIndex index = this->downloadsTab->ui->torrentView->model()->index(row,Download::StartStopButton_Col);
        if(index.isValid())
            emit this->downloadsTab->ui->torrentView->model()->dataChanged(index,index);
        item->setToolTip(Download::StartStopButton_Col,QLatin1String("Start download"));
        item->setText(Download::Status_Col,QLatin1String("Complete"));

        Q_ASSERT(this->notificationTab != NULL);
        if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadComplete_checkBox->isChecked()) {
            Notify* notify = new Notify(QLatin1String("Download Complete"),"File " + item->text(Download::FileName_Col) + " successfully downloaded. Go to " "<b>Downloads</b>" " tab and double-click file you want to open.",10,this);
            notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
            notify->show();
        }
    }

}


int
MainWindow::rowOfClient(TorrentClient *client) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    // Return the row that displays this client's status, or -1 if the
    // client is not known.
    int row = 0;
    foreach (const Job& job, jobs) {
        if (job.client == client)
            return row;
        ++row;
    }
    return -1;
}

static const unsigned long KILOBYTE = 1024;
static const unsigned long MEGABYTE = 1048576;
static const unsigned long GIGABYTE = 1073741824;
static const qint64 TERABYTE = 1099511627776;

void
MainWindow::updateBytes(qint64 bytesReceived,qint64 totalBytes)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    TorrentClient *client = qobject_cast<TorrentClient *>(sender());
    int row = 0;
    if(client)
       row = rowOfClient(client);


    Q_ASSERT(this->downloadsTab != NULL);
    QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(row);
    if (item) {

        QString tmp;

        if(bytesReceived < KILOBYTE){
            tmp = QString::number(bytesReceived) + " B";
        } else if (bytesReceived < MEGABYTE) {
            tmp = QString::number((static_cast<double>(bytesReceived)/KILOBYTE),'f',2)  + " KB";
            } else if (bytesReceived < GIGABYTE) {
                tmp = QString::number((static_cast<double>(bytesReceived)/MEGABYTE),'f',2)  + " MB";
                } else if (bytesReceived < TERABYTE) {
                tmp = QString::number((static_cast<double>(bytesReceived)/GIGABYTE),'f',2)  + " GB";
                } else {
                    tmp = QString::number((static_cast<double>(bytesReceived)/TERABYTE),'f',2)  + " TB";
                }

        tmp += " / ";

        if(totalBytes < KILOBYTE){
            tmp += QString::number(totalBytes) + " B";
        } else if (totalBytes < MEGABYTE) {
            tmp += QString::number((static_cast<double>(totalBytes)/KILOBYTE),'f',2) + " KB";
            } else if (totalBytes < GIGABYTE) {
                tmp += QString::number((static_cast<double>(totalBytes)/MEGABYTE),'f',2) + " MB";
                } else if (totalBytes < TERABYTE) {
                tmp +=  QString::number((static_cast<double>(totalBytes)/GIGABYTE),'f',2) + " GB";
                } else {
                    tmp += QString::number((static_cast<double>(totalBytes)/TERABYTE),'f',2) + " TB";
                }

        item->setText(Download::FileSize_Col, tmp);
    }

}

void
MainWindow::updateProgress(int percent)
{
    qDebug() << "MainWindow::updateProgrss() "  << percent;

     TorrentClient *client = qobject_cast<TorrentClient *>(sender());
     int row = 0;

     if(client)
        row = rowOfClient(client);

     // Update the progressbar.
     Q_ASSERT(this->downloadsTab != NULL);
     QTreeWidgetItem *item = this->downloadsTab->ui->torrentView->topLevelItem(row);
     if (item) {
         qDebug() << "UPDATING !!!!";
         item->setText(Download::Progress_Col, QString::number(percent));
         item->setText(Download::Status_Col,QLatin1String("Downloading ..."));
     }
}

/* FIXME: There is currently two, basically identical startDownload() functions.
 * One for Content-Disposition handling (startDownload(QNetworkReply*&)) and one for normal URL.
 * (startDownload(const QUrl&))
 *
 * Ideally, there should be only one startDownload() function that does it all.
 * Cleanup duplicate code and make it simpler/cleaner */

void
MainWindow::startDownload(QNetworkReply*& reply)
{
    qDebug() << "====================================================";
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "====================================================";

    qDebug() << "BYTES AVAILABLE " << reply->bytesAvailable();
    qDebug() << "ACTUAL LENGTH " << reply->rawHeader("Content-Length");

    QString baseFilename;

    qDebug() << "CONTENT-DISPOSITION " << reply->rawHeader("Content-Disposition");

    if(!reply->rawHeader("Content-Disposition").isEmpty()) {
        baseFilename = QString(reply->rawHeader("Content-Disposition")).replace(QRegularExpression(QLatin1String("(.*filename=)\"(.*)\"")),QLatin1String("\\2"));
    } else {
        baseFilename = reply->url().path().split(QLatin1String("/")).last();
    }

    qDebug() << "FILE IS " << baseFilename;


    QString filterString = this->mimeDatabase.mimeTypeForFile(baseFilename,QMimeDatabase::MatchExtension).filterString();
    filterString += tr(";;Any (*.*)");

    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                               QDir::currentPath() + "/" + baseFilename,
                               filterString);

    /* If trying to download the same file, to same dir(!) and to top of that, same file download is
     * still ongoing, then stop it right there and send notify to user.
     * If the same file download progress has already finished then everything is ok
     * and let the user chance to overwrite file if he/she so wants */

    foreach (const Job& job, jobs) {
        if((job.destinationDirectory == filename) && !job.client->isFinished() && !job.client->isStopped()) {

            Q_ASSERT(this->notificationTab != NULL);
            if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadError_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Already Downloading!"),"Downloading to file " + filename.split("/").last() + " already in progress ...",10,this);
                notify->show();
            }
            return;
        }

    }


    if(!filename.isNull() && !filename.isEmpty()) {
        qDebug() << "FILENAME " << filename;
        qDebug() << "REPLY URL " << reply->url();

        /******************************************************************/
        /* First check is the same file (and to same path) already
         * downloaded and finished. If it is then start() downloading again */
        /******************************************************************/
        foreach (const Job& job, jobs) {
            /* Same file (in same path) already downloaded and ready.
             * Don't add new one to download list. Instead execute already
             * client->start() that is in the list */
            if((job.destinationDirectory == filename) && job.client->isFinished() && job.client->isStopped()) {


                Q_ASSERT(this->notificationTab != NULL);
                if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadStart_checkBox->isChecked()) {
                    Notify* notify = new Notify(QLatin1String("File Already Downloaded "),"Redownloading file " + filename.split("/").last() + " ...",10,this);
                    notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
                    notify->show();
                }

                job.client->start();
                return;
            }
        }
        /******************************************************************/

        client = new TorrentClient(reply,filename,this);

        connect(client,SIGNAL(progressUpdated(int)),this,SLOT(updateProgress(int)));
        connect(client,SIGNAL(downloadedBytes(qint64,qint64)),this,SLOT(updateBytes(qint64,qint64)));
        connect(client,SIGNAL(progressFinished(QNetworkReply*)),this,SLOT(finishedProgress(QNetworkReply*)));
        connect(client,SIGNAL(progressFailed(QNetworkReply*)),this,SLOT(failedProgress(QNetworkReply*)));

        Job job;
        job.client = client;
        job.destinationDirectory = filename;
        jobs << job;

        // Create and add a row in the torrent view for this download.
        Q_ASSERT(this->downloadsTab != NULL);
        QTreeWidgetItem *item = new QTreeWidgetItem(this->downloadsTab->ui->torrentView);

        item->setToolTip(Download::FileName_Col,filename);
        item->setText(Download::FileName_Col, QFileInfo(filename).fileName());
        item->setText(Download::FileSize_Col, 0);
        item->setText(Download::Progress_Col,QLatin1String("0"));
        item->setText(Download::Status_Col,QLatin1String("Starting ..."));
        item->setToolTip(Download::StartStopButton_Col,QLatin1String("Cancel download"));


        Q_ASSERT(this->notificationTab != NULL);
        if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadStart_checkBox->isChecked()) {
            Notify* notify = new Notify(QLatin1String("Download Started"),"File " + item->text(Download::FileName_Col) + " is being downloaded.",10,this);
            notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
            notify->show();
        }

    }

}

void
MainWindow::startDownload(const QUrl& url)
{
    qDebug() << "====================================================";
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "====================================================";
    qDebug() << "FILE IS " << url.path().split("/").last();
    qDebug() << QDir::currentPath() + "/" + url.path().split("/").last();

    QString baseFilename;

    baseFilename = url.path().split(QLatin1String("/")).last();

    QString filterString = this->mimeDatabase.mimeTypeForFile(baseFilename,QMimeDatabase::MatchExtension).filterString();
    filterString += tr(";;Any (*.*)");

    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                               QDir::currentPath() + "/" + baseFilename,
                               filterString);

    /* If trying to download the same file, to same dir(!) and to top of that, same file download is
     * still ongoing, then stop it right there and send notify to user.
     * If the same file download progress has already finished then everything is ok
     * and let the user chance to overwrite file if he/she so wants */
    foreach (const Job& job, jobs) {
        if((job.destinationDirectory == filename) && !job.client->isFinished() && !job.client->isStopped()) {


            Q_ASSERT(this->notificationTab != NULL);
            if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadError_checkBox->isChecked()) {
                Notify* notify = new Notify(QLatin1String("Already Downloading!"),"Downloading to file " + filename.split("/").last() + " already in progress ...",10,this);
                notify->show();
            }
            return;
        }

    }


    if(!filename.isNull() && !filename.isEmpty()) {
        qDebug() << "FILENAME " << filename;
        qDebug() << "URL " << url;
        /******************************************************************/
        /* First check is the same file (and to same path) already
         * downloaded and finished. If it is then start() downloading again */
        /******************************************************************/
        foreach (const Job& job, jobs) {
            /* Same file (in same path) already downloaded and ready.
             * Don't add new one to download list. Instead execute already
             * client->start() that is in the list */
            if((job.destinationDirectory == filename) && job.client->isFinished() && job.client->isStopped()) {


                Q_ASSERT(this->notificationTab != NULL);
                if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadStart_checkBox->isChecked()) {
                    Notify* notify = new Notify(QLatin1String("File Already Downloaded "),"Redownloading file " + filename.split("/").last() + " ...",10,this);
                    notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
                    notify->show();
                }

                job.client->start();
                return;
            }
        }
        /******************************************************************/
        client = new TorrentClient(url,filename,this);

        connect(client,SIGNAL(progressUpdated(int)),this,SLOT(updateProgress(int)));
        connect(client,SIGNAL(downloadedBytes(qint64,qint64)),this,SLOT(updateBytes(qint64,qint64)));
        connect(client,SIGNAL(progressFinished(QNetworkReply*)),this,SLOT(finishedProgress(QNetworkReply*)));
        connect(client,SIGNAL(progressFailed(QNetworkReply*)),this,SLOT(failedProgress(QNetworkReply*)));


        Job job;
        job.client = client;
        job.destinationDirectory = filename;
        jobs << job;

        // Create and add a row in the torrent view for this download.
        Q_ASSERT(this->downloadsTab != NULL);
        QTreeWidgetItem *item = new QTreeWidgetItem(this->downloadsTab->ui->torrentView);

        item->setToolTip(Download::FileName_Col,filename);
        item->setText(Download::FileName_Col, QFileInfo(filename).fileName());
        item->setText(Download::FileSize_Col, 0);
        item->setText(Download::Progress_Col,QLatin1String("0"));
        item->setText(Download::Status_Col,QLatin1String("Starting ..."));
        item->setToolTip(Download::StartStopButton_Col,QLatin1String("Cancel download"));

        Q_ASSERT(this->notificationTab != NULL);
        if(this->notificationTab->ui->notifications_checkBox->isChecked() && this->notificationTab->ui->notifyDownloadStart_checkBox->isChecked()) {
            Notify* notify = new Notify(QLatin1String("Download Started"),"File " + item->text(Download::FileName_Col) + " is being downloaded.",10,this);
            notify->setTimeoutState(this->notificationTab->ui->showNotifyTimeout_checkBox->isChecked());
            notify->show();
        }
    }
}

void
MainWindow::slot_unsupportedContent(QNetworkReply * reply)
{
    qDebug() << "====================================================";
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "====================================================";
    qDebug() << "BYTES AVAILABLE " << reply->bytesAvailable();
    qDebug() << "ACTUAL LENGTH " << reply->rawHeader("Content-Length");
    qDebug() << "CONTENT-DISPOSITION " << reply->rawHeader("Content-Disposition");

    if(!reply->rawHeader("Content-Disposition").isEmpty()) {
        startDownload(reply);
    } else {
        startDownload(reply->url());
    }
}


void
MainWindow::slot_downloadRequested(const QNetworkRequest & request)
{
    startDownload(request.url());    
}

/* ============================================== */
void
MainWindow::on_actionZoom_in_triggered()
{
    QWebView* w = this->currentTab()->getWebView();
    if(w) {
        if(w->zoomFactor() < 3.0) {
            w->setZoomFactor(w->zoomFactor() + 0.1);
        }
    }
}

void
MainWindow::on_actionZoom_Out_triggered()
{
    QWebView* w = this->currentTab()->getWebView();
    if(w) {
        if(w->zoomFactor() > 0.3) {
            w->setZoomFactor(w->zoomFactor() - 0.1);
        }
    }
}

void
MainWindow::on_actionPrint_triggered()
{
        QPrinter p;
        QPrintPreviewDialog preview(&p);
        connect(&preview,SIGNAL(paintRequested(QPrinter*)),this->currentTab()->getWebView(),SLOT(print(QPrinter*)));
        preview.setWindowTitle(this->currentTab()->getWebPage()->mainFrame()->title());
        preview.exec();
}

void
MainWindow::on_actionPrint_as_PDF_triggered()
{
    /* PDF printing */
    QPrinter p(QPrinter::HighResolution);
    p.setOutputFormat(QPrinter::PdfFormat);
    QString fileName = this->currentTab()->getWebPage()->mainFrame()->title() + ".pdf";
    p.setOutputFileName(fileName.remove(QRegularExpression(QLatin1String("[\\\/\:\*\?\"\<\>\|]."))));
    QPrintPreviewDialog preview(&p);
    connect(&preview,SIGNAL(paintRequested(QPrinter*)),this->currentTab()->getWebView(),SLOT(print(QPrinter*)));
    preview.exec();
}


void
MainWindow::on_actionClose_Tab_triggered()
{
    this->closeTab(this->ui->Browser_tabWidget->currentIndex());
}

void
MainWindow::on_actionOpen_New_Tab_triggered()
{
    this->addTab();

    qDebug() << " NEW EMPTY (about:blank) TAB";
}


void
MainWindow::on_actionAbout_CyberDragon_triggered()
{
    Dialog* d = new Dialog(this);
    d->exec();
}

/* ====================================================== */

void
MainWindow::loadCiphers()
{

    Q_ASSERT(this->encryptionTab != NULL);
        QSslConfiguration   sslConfig;
        QList<QSslCipher>   ciphers;
        QList<QSslCipher>   ciphers2;
        sslConfig = QSslConfiguration::defaultConfiguration();
        this->encryptionTab->ui->Ciphers_treeWidget->clear();

        QSettings settings(this->settingsFile, QSettings::IniFormat);

        settings.beginGroup(QLatin1String("encryption"));
        int n = settings.beginReadArray(QLatin1String("cipher"));
        for (int i = 0; i < n; i++) {
            settings.setArrayIndex(i);

            struct cipherList   cipher;
            cipher.name = settings.value(QLatin1String("name")).toString();
            cipher.pfs = settings.value(QLatin1String("pfs")).toBool();
            cipher.protocol = settings.value(QLatin1String("protocol")).toString();
            cipher.enabled = settings.value(QLatin1String("enabled")).toBool();

            QTreeWidgetItem*    item = new QTreeWidgetItem(this->encryptionTab->ui->Ciphers_treeWidget);

            QSsl::SslProtocol   protocol;

            if(cipher.protocol == QLatin1String("TLSv1.2"))
                protocol = QSsl::TlsV1_2;
            if(cipher.protocol == QLatin1String("SSLv3"))
                protocol = QSsl::SslV3;

            ciphers2.append(QSslCipher(cipher.name,protocol));

            if(cipher.enabled) {
                item->setCheckState(0,Qt::Checked);
            } else {
                item->setCheckState(0,Qt::Unchecked);
            }

            item->setText(0,ciphers2.at(i).name());
            item->setText(1,(cipher.pfs ? QLatin1String("Yes") : QLatin1String("No")));

            item->setTextColor(1,(cipher.pfs ? MediumSpringGreen : ChilliPepper));
            item->setText(2,ciphers2.at(i).protocolString());
            if(ciphers2.at(i).protocolString() == QLatin1String("TLSv1.2"))
                item->setTextColor(2,MediumSpringGreen);
            if(ciphers2.at(i).protocolString() == QLatin1String("SSLv3"))
                item->setTextColor(2,ChilliPepper);

            item->setText(3,ciphers2.at(i).authenticationMethod());

            item->setText(4,ciphers2.at(i).encryptionMethod());
            if(ciphers2.at(i).encryptionMethod().indexOf(QLatin1String("AES")) == 0) {
                item->setTextColor(4,MediumSpringGreen);
            }
            if(ciphers2.at(i).encryptionMethod().indexOf(QLatin1String("RC4")) == 0) {
                item->setTextColor(4,DarkOrange);
            }
            item->setText(5,ciphers2.at(i).keyExchangeMethod());


        }
        settings.endArray();
        settings.endGroup();

        register size_t n2 = this->encryptionTab->ui->Ciphers_treeWidget->topLevelItemCount();
        for(register unsigned int i = 0;i < n2;++i) {
            if(this->encryptionTab->ui->Ciphers_treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked) {
                QSsl::SslProtocol   protocol;

                if(this->encryptionTab->ui->Ciphers_treeWidget->topLevelItem(i)->text(2) == QLatin1String("TLSv1.2"))
                    protocol = QSsl::TlsV1_2;
                if(this->encryptionTab->ui->Ciphers_treeWidget->topLevelItem(i)->text(2) == QLatin1String("SSLv3"))
                    protocol = QSsl::SslV3;

                ciphers.append(QSslCipher(this->encryptionTab->ui->Ciphers_treeWidget->topLevelItem(i)->text(0),protocol));
            }


        }

        for(register unsigned int i = 0;i < 6;++i) {
            this->encryptionTab->ui->Ciphers_treeWidget->resizeColumnToContents(i);
        }

        sslConfig.setCiphers(ciphers);
        QSslConfiguration::setDefaultConfiguration(sslConfig);

}


void
MainWindow::slot_FindText()
{
    QWebView*   webview = this->currentTab()->getWebView();

    webview->findText(QLatin1String(""),this->findOptions);    // Zero previous search
    this->findOptions = 0;

    if(this->findHighlightAll->isChecked())
        this->findOptions  |= QWebPage::HighlightAllOccurrences;

    if(this->findCaseSensitive->isChecked())
        this->findOptions  |=  QWebPage::FindCaseSensitively;

    webview->findText(this->findBox->text(),this->findOptions );

}

void
MainWindow::slot_FindText(const QString& text)
{
    QWebView*   webview = this->currentTab()->getWebView();

    webview->findText(QLatin1String(""),this->findOptions);    // Zero previous search
    this->findOptions = 0;

    if(this->findHighlightAll->isChecked())
        this->findOptions  |= QWebPage::HighlightAllOccurrences;

    if(this->findCaseSensitive->isChecked())
        this->findOptions  |=  QWebPage::FindCaseSensitively;

    webview->findText(text,this->findOptions );
}

void
MainWindow::on_actionFindForward_triggered()
{
    QWebView*   webview = this->currentTab()->getWebView();
    this->findOptions = 0;

    if(this->findHighlightAll->isChecked())
        this->findOptions  |= QWebPage::HighlightAllOccurrences;

    if(this->findCaseSensitive->isChecked())
        this->findOptions  |=  QWebPage::FindCaseSensitively;

    webview->findText(this->findBox->text(),this->findOptions );
}

void
MainWindow::on_actionFindBackward_triggered()
{
    QWebView*   webview = this->currentTab()->getWebView();

    this->findOptions = 0;

    if(this->findHighlightAll->isChecked())
        this->findOptions  |= QWebPage::HighlightAllOccurrences;

    if(this->findCaseSensitive->isChecked())
        this->findOptions  |=  QWebPage::FindCaseSensitively;

    this->findOptions |= QWebPage::FindBackward;

    webview->findText(this->findBox->text(),this->findOptions );


}

void
MainWindow::on_actionFind_triggered()
{
    this->findBox->setFocus();
}


void MainWindow::on_ReloadMap_pushButton_clicked()
{
    Browser*    browser = this->currentTab();
    if(browser) {
        browser->getServerInfo()->loadGoogleMaps();
    }
}
