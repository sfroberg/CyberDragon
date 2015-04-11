/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "general.h"
#include "ui_general.h"

#include "ui_mainwindow.h"

#include "browser.h"
#include "ui_browser.h"

#include <QDebug>
#include <QWebSettings>
#include <QSettings>
#include <QFile>
#include <QTime>

inline static
quint32 random(quint32 low, quint32 high)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  (qrand() % ((high + 1) - low) + low);
}

general::general(MainWindow* m, QWidget *parent) :
    QWidget(parent),settingsFile(QLatin1String("config.ini")),mainWindow(m),
    ui(new Ui::general)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    ui->setupUi(this);

    /* Initialize random number generator seed needed by proxy random hopping */
    qsrand((uint)QTime::currentTime().msec());

    this->loadSettings();

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled,true);
    QWebSettings::setIconDatabasePath("./iconDatabase");

    this->loadUserAgents();

    this->settings = new QButtonGroup(this);
    this->settings->setExclusive(false);

    this->settings->addButton(this->ui->AutoLoadImages_checkBox,QWebSettings::AutoLoadImages);
    this->settings->addButton(this->ui->DnsPrefetch_checkBox,QWebSettings::DnsPrefetchEnabled);
    this->settings->addButton(this->ui->JavaScript_checkBox,QWebSettings::JavascriptEnabled);
    this->settings->addButton(this->ui->JavaScriptCanAccessClipboard_checkBox,QWebSettings::JavascriptCanAccessClipboard);
    this->settings->addButton(this->ui->JavaScriptCanOpenWindows_checkBox,QWebSettings::JavascriptCanOpenWindows);
    this->settings->addButton(this->ui->Plugins_checkBox,QWebSettings::PluginsEnabled);
    this->settings->addButton(this->ui->PrivateBrowsing_checkBox,QWebSettings::PrivateBrowsingEnabled);
    this->settings->addButton(this->ui->SpatialNavigation_checkBox,QWebSettings::SpatialNavigationEnabled);
    this->settings->addButton(this->ui->LinksIncludedInFocusChain_checkBox,QWebSettings::LinksIncludedInFocusChain);
    this->settings->addButton(this->ui->ZoomTextOnly_checkBox,QWebSettings::ZoomTextOnly);
    this->settings->addButton(this->ui->PrintElementBackgrounds_checkBox,QWebSettings::PrintElementBackgrounds);
    this->settings->addButton(this->ui->OfflineStorageDatabase_checkBox,QWebSettings::OfflineStorageDatabaseEnabled);
    this->settings->addButton(this->ui->OfflineWebApplicationCache_checkBox,QWebSettings::OfflineWebApplicationCacheEnabled);
    this->settings->addButton(this->ui->LocalStorage_checkBox,QWebSettings::LocalStorageEnabled);
    this->settings->addButton(this->ui->LocalContentCanAccessRemoteUrls_checkBox,QWebSettings::LocalContentCanAccessRemoteUrls);
    this->settings->addButton(this->ui->LocalContentCanAccessFileUrls_checkBox,QWebSettings::LocalContentCanAccessFileUrls);
    this->settings->addButton(this->ui->XSSAuditing_checkBox,QWebSettings::XSSAuditingEnabled);
    this->settings->addButton(this->ui->WebGL_checkBox,QWebSettings::WebGLEnabled);

/* These need QGrahpicsWebView for working. Disable for now
    this->settings->addButton(this->ui->AcceleratedCompositingEnabled_checkBox,QWebSettings::AcceleratedCompositingEnabled);
    this->settings->addButton(this->ui->TiledBackingStoreEnabled_checkBox,QWebSettings::TiledBackingStoreEnabled);
*/
    /* AFAIK, only used with touch devises like smartphones. Disable for now */
//    this->settings->addButton(this->ui->FrameFlattening_checkBox,QWebSettings::FrameFlatteningEnabled);

    connect(this->settings,SIGNAL(buttonClicked(int)),this,SLOT(buttonClicked(int)));

    this->httpReferer = new QButtonGroup(this);
    this->httpReferer->addButton(this->ui->DoNotTouchHTTPReferer_radioButton,HTTPReferer::DoNotTouch);
    this->httpReferer->addButton(this->ui->RemoveHTTPReferer_radioButton,HTTPReferer::Remove);
    this->httpReferer->addButton(this->ui->SameAsCurrentUrlHTTPReferer_radioButton,HTTPReferer::SameAsCurrentUrl);
    this->httpReferer->addButton(this->ui->CustomHTTPReferer_radioButton,HTTPReferer::CustomHTTPReferer);


}

general::~general()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->saveSettings();
    delete ui;
}

void
general::buttonClicked(int id)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QWebSettings::globalSettings()->setAttribute(static_cast<enum QWebSettings::WebAttribute>(id),this->settings->button(id)->isChecked());
}

void
general::loadUserAgents()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    /* =========================*/
    /* Load User Agents       */
    /* =========================*/

        QFile   userAgent(QLatin1String("useragents.txt"));
        if(userAgent.open(QIODevice::ReadOnly | QIODevice::Text)) {

            this->ui->UserAgent_treeWidget->sortByColumn(0,Qt::AscendingOrder);
            while(!userAgent.atEnd()){
                QString line = userAgent.readLine().trimmed();
                if(!line.isEmpty()) {
                    QStringList list = line.split(QLatin1String("\t"),QString::SkipEmptyParts);
                    QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->UserAgent_treeWidget,list);
                }
            }
            userAgent.close();
        }
        this->ui->UserAgent_treeWidget->resizeColumnToContents(0);
        this->ui->UserAgent_treeWidget->resizeColumnToContents(1);
}

void
general::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("general_settings"));

    this->ui->AutoLoadImages_checkBox->setChecked(settings.value(QLatin1String("auto_load_images")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AutoLoadImages,settings.value(QLatin1String("auto_load_images")).toBool());

    this->ui->DnsPrefetch_checkBox->setChecked(settings.value(QLatin1String("dns_prefetch")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::DnsPrefetchEnabled,settings.value(QLatin1String("dns_prefetch")).toBool());

    this->ui->JavaScript_checkBox->setChecked(settings.value(QLatin1String("javascript")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled,settings.value(QLatin1String("javascript")).toBool());

    this->ui->JavaScriptCanOpenWindows_checkBox->setChecked(settings.value(QLatin1String("javascript_can_open_windows")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanOpenWindows,settings.value(QLatin1String("javascript_can_open_windows")).toBool());

    this->ui->JavaScriptCanAccessClipboard_checkBox->setChecked(settings.value(QLatin1String("javascript_can_access_clipboard")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard,settings.value(QLatin1String("javascript_can_access_clipboard")).toBool());

    this->ui->Plugins_checkBox->setChecked(settings.value(QLatin1String("plugins")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled,settings.value(QLatin1String("plugins")).toBool());

    this->ui->PrivateBrowsing_checkBox->setChecked(settings.value(QLatin1String("private_browsing")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PrivateBrowsingEnabled,settings.value(QLatin1String("private_browsing")).toBool());

    this->ui->SpatialNavigation_checkBox->setChecked(settings.value(QLatin1String("spatial_navigation")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::SpatialNavigationEnabled,settings.value(QLatin1String("spatial_navigation")).toBool());

    this->ui->LinksIncludedInFocusChain_checkBox->setChecked(settings.value(QLatin1String("links_included_in_focus_chain")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LinksIncludedInFocusChain,settings.value(QLatin1String("links_included_in_focus_chain")).toBool());

    this->ui->ZoomTextOnly_checkBox->setChecked(settings.value(QLatin1String("zoom_text_only")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::ZoomTextOnly,settings.value(QLatin1String("zoom_text_only")).toBool());

    this->ui->PrintElementBackgrounds_checkBox->setChecked(settings.value(QLatin1String("print_element_background")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PrintElementBackgrounds,settings.value(QLatin1String("print_element_background")).toBool());

    this->ui->OfflineStorageDatabase_checkBox->setChecked(settings.value(QLatin1String("offline_storage_database")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled,settings.value(QLatin1String("offline_storage_database")).toBool());

    this->ui->OfflineWebApplicationCache_checkBox->setChecked(settings.value(QLatin1String("offline_web_application_cache")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled,settings.value(QLatin1String("offline_web_application_cache")).toBool());

    this->ui->LocalStorage_checkBox->setChecked(settings.value(QLatin1String("local_storage")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalStorageEnabled,settings.value(QLatin1String("local_storage")).toBool());

    this->ui->LocalContentCanAccessRemoteUrls_checkBox->setChecked(settings.value(QLatin1String("local_content_can_access_remote_urls")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls,settings.value(QLatin1String("local_content_can_access_remote_urls")).toBool());

    this->ui->LocalContentCanAccessFileUrls_checkBox->setChecked(settings.value(QLatin1String("local_content_can_access_file_urls")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls,settings.value(QLatin1String("local_content_can_access_file_urls")).toBool());

    this->ui->XSSAuditing_checkBox->setChecked(settings.value(QLatin1String("xss_auditing")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::XSSAuditingEnabled,settings.value(QLatin1String("xss_auditing")).toBool());

/* These need QGrahpicsWebView for working. Disable for now
    this->ui->AcceleratedCompositingEnabled_checkBox->setChecked(settings.value("accelerated_compositing").toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::AcceleratedCompositingEnabled,settings.value("accelerated_compositing").toBool());

    this->ui->TiledBackingStoreEnabled_checkBox->setChecked(settings.value("tiled_backing_store").toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::TiledBackingStoreEnabled,settings.value("tiled_backing_store").toBool());
*/

 /* AFAIK, only used with touch devises like smartphones. Disable for now */
/*    this->ui->FrameFlattening_checkBox->setChecked(settings.value(QLatin1String("frame_flattening")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::FrameFlatteningEnabled,settings.value(QLatin1String("frame_flattening")).toBool());*/

    this->ui->WebGL_checkBox->setChecked(settings.value(QLatin1String("webgl")).toBool());
    QWebSettings::globalSettings()->setAttribute(QWebSettings::WebGLEnabled,false);

    if(!settings.contains(QLatin1String("home_page"))) {
        this->ui->HomePage_lineEdit->setText(QLatin1String("https://startpage.com"));
        this->ui->HomePage_lineEdit->setCursorPosition(0);
    } else {
        this->ui->HomePage_lineEdit->setText(settings.value(QLatin1String("home_page")).toString());
        this->ui->HomePage_lineEdit->setCursorPosition(0);
    }

    this->ui->HttpPipeline_checkBox->setChecked(settings.value(QLatin1String("http_pipelining")).toBool());

    switch(settings.value("http_referer").toUInt()) {
        case    HTTPReferer::CustomHTTPReferer:
        {
            this->ui->CustomHTTPReferer_radioButton->setChecked(true);
            this->ui->CustomHTTPReferer_radioButton->toggled(true);
            this->ui->CustomHTTPReferer_lineEdit->setText(settings.value("http_referer_custom").toString());
        }
        break;
        case    HTTPReferer::SameAsCurrentUrl:
            this->ui->SameAsCurrentUrlHTTPReferer_radioButton->setChecked(true);
        break;
        case    HTTPReferer::Remove:
            this->ui->RemoveHTTPReferer_radioButton->setChecked(true);
        break;
        case    HTTPReferer::DoNotTouch:
        default:
            this->ui->DoNotTouchHTTPReferer_radioButton->setChecked(true);
    }


    this->ui->UserAgent_lineEdit->setText(settings.value(QLatin1String("useragent")).toString());
    this->ui->UserAgent_lineEdit->setCursorPosition(0);
    this->ui->UserAgentRandomly_radioButton->setChecked(settings.value(QLatin1String("useragent_random")).toBool());
    this->ui->PreventETags_checkBox->setChecked(settings.value(QLatin1String("prevent_etags_tracking")).toBool());    
    this->ui->DisableGoogleSearchAutocomplete_checkBox->setChecked(settings.value(QLatin1String("disable_google_search_autocomplete")).toBool());
    this->ui->PreventGoogleTrackingRedirectLinks_checkBox->setChecked(settings.value(QLatin1String("prevent_google_tracking_redirect_links")).toBool());

    settings.endGroup();
}

void
general::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("general_settings"));
    settings.setValue(QLatin1String("auto_load_images"),this->ui->AutoLoadImages_checkBox->isChecked());
    settings.setValue(QLatin1String("dns_prefetch"),this->ui->DnsPrefetch_checkBox->isChecked());
    settings.setValue(QLatin1String("javascript"),this->ui->JavaScript_checkBox->isChecked());
    settings.setValue(QLatin1String("javascript_can_open_windows"),this->ui->JavaScriptCanOpenWindows_checkBox->isChecked());
    settings.setValue(QLatin1String("javascript_can_access_clipboard"),this->ui->JavaScriptCanAccessClipboard_checkBox->isChecked());
    settings.setValue(QLatin1String("plugins"),this->ui->Plugins_checkBox->isChecked());
    settings.setValue(QLatin1String("private_browsing"),this->ui->PrivateBrowsing_checkBox->isChecked());
    settings.setValue(QLatin1String("spatial_navigation"),this->ui->SpatialNavigation_checkBox->isChecked());
    settings.setValue(QLatin1String("links_included_in_focus_chain"),this->ui->LinksIncludedInFocusChain_checkBox->isChecked());
    settings.setValue(QLatin1String("zoom_text_only"),this->ui->ZoomTextOnly_checkBox->isChecked());
    settings.setValue(QLatin1String("print_element_background"),this->ui->PrintElementBackgrounds_checkBox->isChecked());
    settings.setValue(QLatin1String("offline_storage_database"),this->ui->OfflineStorageDatabase_checkBox->isChecked());
    settings.setValue(QLatin1String("offline_web_application_cache"),this->ui->OfflineWebApplicationCache_checkBox->isChecked());
    settings.setValue(QLatin1String("local_storage"),this->ui->LocalStorage_checkBox->isChecked());
    settings.setValue(QLatin1String("local_content_can_access_remote_urls"),this->ui->LocalContentCanAccessRemoteUrls_checkBox->isChecked());
    settings.setValue(QLatin1String("local_content_can_access_file_urls"),this->ui->LocalContentCanAccessFileUrls_checkBox->isChecked());
    settings.setValue(QLatin1String("xss_auditing"),this->ui->XSSAuditing_checkBox->isChecked());
    settings.setValue(QLatin1String("accelerated_compositing"),this->ui->AcceleratedCompositing_checkBox->isChecked());
    settings.setValue(QLatin1String("tiled_backing_store"),this->ui->TiledBackingStore_checkBox->isChecked());
    settings.setValue(QLatin1String("frame_flattening"),this->ui->FrameFlattening_checkBox->isChecked());
    settings.setValue(QLatin1String("webgl"),this->ui->WebGL_checkBox->isChecked());
    settings.setValue(QLatin1String("home_page"),this->ui->HomePage_lineEdit->text());
    settings.setValue(QLatin1String("http_pipelining"),this->ui->HttpPipeline_checkBox->isChecked());
    settings.setValue(QLatin1String("http_referer"),this->httpReferer->checkedId());
    settings.setValue(QLatin1String("http_referer_custom"),this->ui->CustomHTTPReferer_lineEdit->text());
    settings.setValue(QLatin1String("useragent"),this->ui->UserAgent_lineEdit->text());
    settings.setValue(QLatin1String("useragent_random"),this->ui->UserAgentRandomly_radioButton->isChecked());
    settings.setValue(QLatin1String("prevent_etags_tracking"),this->ui->PreventETags_checkBox->isChecked());
    settings.setValue(QLatin1String("disable_google_search_autocomplete"),this->ui->DisableGoogleSearchAutocomplete_checkBox->isChecked());
    settings.setValue(QLatin1String("prevent_google_tracking_redirect_links"),this->ui->PreventGoogleTrackingRedirectLinks_checkBox->isChecked());

    settings.endGroup();
}

void
general::on_UserAgent_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(current) {
        this->ui->UserAgent_lineEdit->setText(current->text(1));
        this->ui->UserAgent_lineEdit->setCursorPosition(0);
    }
}

void
general::randomUserAgent()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    int index = random(0,this->ui->UserAgent_treeWidget->topLevelItemCount()-1);
    this->ui->UserAgent_treeWidget->setCurrentItem(this->ui->UserAgent_treeWidget->topLevelItem(index));
}

void general::on_UserAgentManually_radioButton_toggled(bool checked)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->UserAgent_treeWidget->setEnabled(true);
}

void general::on_UserAgentRandomly_radioButton_toggled(bool checked)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->UserAgent_treeWidget->setEnabled(false);
}

void general::on_SetHomePage_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(this->mainWindow) {
        Browser*    browser = qobject_cast<Browser*>(this->mainWindow->ui->Browser_tabWidget->currentWidget());
        if(browser)         {
            this->ui->HomePage_lineEdit->setText(browser->ui->URL_comboBox->lineEdit()->text());
            this->ui->HomePage_lineEdit->setCursorPosition(0);
        }
    }
}
