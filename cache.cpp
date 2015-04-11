/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "cache.h"
#include "ui_cache.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include "browser.h"

#include <QFileDialog>
#include <QSettings>

Cache::Cache(MainWindow *parent) :
    QWidget(parent),mainWindow(parent),settingsFile(QLatin1String("config.ini")),
    ui(new Ui::cache)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);

    this->cacheSettings = new QButtonGroup(this);

    this->cacheSettings->addButton(this->ui->noCache_radioButton,DiskCache::NoCache);
    this->cacheSettings->addButton(this->ui->enableCache_radioButton,DiskCache::CacheEnabled);
    this->cacheSettings->addButton(this->ui->allwaysCache_radioButton,DiskCache::OfflineCache);

    connect(this->cacheSettings,SIGNAL(buttonClicked(int)),this,SLOT(cacheButtonClicked(int)));

    this->loadSettings();
}

Cache::~Cache()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->saveSettings();

    if(this->ui->clearCache_checkBox->isChecked()) {
        this->on_clearCache_pushButton_clicked();
    }

    delete ui;
}

void
Cache::setMainWindow(MainWindow* m)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->mainWindow = m;
}

MainWindow*
Cache::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    return this->mainWindow;
}


void
Cache::cacheButtonClicked(int id)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    switch(id) {
        case    DiskCache::OfflineCache:
        case    DiskCache::NoCache:
        {
            this->ui->cacheDir_lineEdit->setEnabled(false);
            this->ui->cacheSize_spinBox->setEnabled(false);
            this->ui->setCache_pushButton->setEnabled(false);
            this->ui->clearCache_checkBox->setEnabled(false);
            this->ui->clearCache_pushButton->setEnabled(false);
            this->ui->cacheDir_pushButton->setEnabled(false);
        }
        break;
        default:
            this->ui->cacheDir_lineEdit->setEnabled(true);
            this->ui->cacheSize_spinBox->setEnabled(true);
            this->ui->setCache_pushButton->setEnabled(true);
            this->ui->clearCache_checkBox->setEnabled(true);
            this->ui->clearCache_pushButton->setEnabled(true);
            this->ui->cacheDir_pushButton->setEnabled(true);
    }
    this->ui->cacheStatus_label->setText(this->getCache());

}

QString
Cache::getCache()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Browser* browser = this->getMainWindow()->currentTab();

    if(browser)
        return browser->getCache();
    else
        return  QString();
}

void
Cache::on_setCache_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

          register size_t n = this->mainWindow->ui->Browser_tabWidget->count();
          for(register unsigned int i = 0;i < n;++i) {
              Browser* browser = qobject_cast<Browser*>(this->mainWindow->ui->Browser_tabWidget->widget(i));
              if(browser) {
                  qDebug() << "Setting CACHE FOR TAB " << i+1;
                  browser->setCache();
              }
          }
}

void
Cache::on_cacheDir_pushButton_clicked()
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    QDir::currentPath(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);


    if(!dir.isEmpty() && !dir.isNull()) {
        this->ui->cacheDir_lineEdit->setText(dir);
    }

}

void
Cache::on_clearCache_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    register size_t n = this->mainWindow->ui->Browser_tabWidget->count();
    for(register unsigned int i = 0;i < n;++i) {
        Browser* browser = qobject_cast<Browser*>(this->mainWindow->ui->Browser_tabWidget->widget(i));
        if(browser) {
            qDebug() << "CLEARING CACHE FOR TAB " << i+1;
            browser->clearCache();
        }
    }

}


void
Cache::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup("cache");
    if(settings.contains("cache_type")) {                
        switch(settings.value("cache_type").toUInt()) {
            case    DiskCache::NoCache:
            {
                this->ui->noCache_radioButton->click();
            }
            break;
            case    DiskCache::CacheEnabled:
            {
                this->ui->enableCache_radioButton->click();
            }
            break;
            case    DiskCache::OfflineCache:
            {
                this->ui->allwaysCache_radioButton->click();
            }
            break;
            default:
            {
                this->ui->enableCache_radioButton->click();
            }
        }
    }

    this->ui->cacheDir_lineEdit->setText(settings.value("cache_dir").toString());
    this->ui->cacheSize_spinBox->setValue(settings.value("cache_size").toUInt());
    this->ui->clearCache_checkBox->setChecked(settings.value("cache_clear_on_exit").toBool());
    settings.endGroup();

}

void
Cache::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup("cache");

    if(this->ui->noCache_radioButton->isChecked())
        settings.setValue("cache_type",DiskCache::NoCache);

    if(this->ui->enableCache_radioButton->isChecked())
        settings.setValue("cache_type",DiskCache::CacheEnabled);

    if(this->ui->allwaysCache_radioButton->isChecked())
        settings.setValue("cache_type",DiskCache::OfflineCache);

    settings.setValue("cache_dir",this->ui->cacheDir_lineEdit->text());
    settings.setValue("cache_size",this->ui->cacheSize_spinBox->value());
    settings.setValue("cache_clear_on_exit",this->ui->clearCache_checkBox->isChecked());

    settings.endGroup();

}


void
Cache::update()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    cacheButtonClicked(this->cacheSettings->checkedId());
    this->ui->setCache_pushButton->click();

}

