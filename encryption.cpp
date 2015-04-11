/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "encryption.h"
#include "ui_encryption.h"

#include "browser.h"
#include "ui_browser.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "general.h"
#include "ui_general.h"

#include <QSettings>
#include <QSslConfiguration>
#include <QSslCipher>

#include <QTreeWidgetItem>
#include <QUrl>

encryption::encryption(MainWindow* m, QWidget *parent) :
    QWidget(parent),mainWindow(m),
    settingsFile(QLatin1String("config.ini")), ui(new Ui::encryption)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    ui->setupUi(this);
    this->loadSettings();
}

encryption::~encryption()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->saveSettings();
    delete ui;
}


void
encryption::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("encryption"));

    this->ui->HTTPSEnforcing_checkBox->setChecked(settings.value(QLatin1String("https_enforcing")).toBool());
    this->ui->MCB_checkBox->setChecked(settings.value(QLatin1String("block_mixed_content")).toBool());
    int size = settings.beginReadArray(QLatin1String("allowed_mixed_content"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->DisableMCB_treeWidget);
        item->setCheckState(0,(settings.value(QLatin1String("enabled")).toBool() ? Qt::Checked : Qt::Unchecked));
        item->setFlags(QFlag(settings.value(QLatin1String("flags")).toInt()));
        item->setText(0,settings.value(QLatin1String("url")).toString());
    }
    settings.endArray();
    settings.endGroup();
}

void
encryption::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("encryption"));

    settings.setValue(QLatin1String("https_enforcing"),this->ui->HTTPSEnforcing_checkBox->isChecked());
    settings.setValue(QLatin1String("block_mixed_content"),this->ui->MCB_checkBox->isChecked());
    settings.remove(QLatin1String("allowed_mixed_content"));

    settings.beginWriteArray(QLatin1String("allowed_mixed_content"));
    for (int i = 0; i < this->ui->DisableMCB_treeWidget->topLevelItemCount(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("enabled"), (this->ui->DisableMCB_treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked ? true : false));
        settings.setValue(QLatin1String("flags"),(int)this->ui->DisableMCB_treeWidget->topLevelItem(i)->flags());
        settings.setValue(QLatin1String("url"),this->ui->DisableMCB_treeWidget->topLevelItem(i)->text(0));
    }
    settings.endArray();

    settings.beginWriteArray(QLatin1String("cipher"));
    for (int i = 0; i < this->ui->Ciphers_treeWidget->topLevelItemCount(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("name"),this->ui->Ciphers_treeWidget->topLevelItem(i)->text(0));
        settings.setValue(QLatin1String("pfs"),(this->ui->Ciphers_treeWidget->topLevelItem(i)->text(1) == QLatin1String("Yes") ? true : false));
        settings.setValue(QLatin1String("protocol"),this->ui->Ciphers_treeWidget->topLevelItem(i)->text(2));
        settings.setValue(QLatin1String("enabled"),(this->ui->Ciphers_treeWidget->topLevelItem(i)->checkState(0) == Qt::Checked));
    }
    settings.endArray();
    settings.endGroup();
}

void encryption::on_MoveCipherUp_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QTreeWidgetItem*    item = this->ui->Ciphers_treeWidget->currentItem();
    if(item) {

        int  row = this->ui->Ciphers_treeWidget->currentIndex().row();

        if (item && row > 0)
        {
                this->ui->Ciphers_treeWidget->takeTopLevelItem(row);
                this->ui->Ciphers_treeWidget->insertTopLevelItem(row - 1,item);
                this->ui->Ciphers_treeWidget->setCurrentItem(item);


            QList<QSslCipher>   ciphers;
            register size_t n = this->ui->Ciphers_treeWidget->topLevelItemCount();
            for(register unsigned int i = 0;i < n;++i) {
                QTreeWidgetItem*    item = this->ui->Ciphers_treeWidget->topLevelItem(i);
                if(item->checkState(0) == Qt::Checked) {
                    QSsl::SslProtocol   protocol;
                    if(item->text(2) == QLatin1String("TLSv1.2")) {
                        protocol = QSsl::TlsV1_2;
                    }
                    if(item->text(2) == QLatin1String("SSLv3")) {
                        protocol = QSsl::SslV3;
                    }

                    ciphers.append(QSslCipher(item->text(0),protocol));
                }
            }

            QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
            sslConfig.setCiphers(ciphers);
            QSslConfiguration::setDefaultConfiguration(sslConfig);

        }
    }

}

void encryption::on_MoveCipherDown_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QTreeWidgetItem*    item = this->ui->Ciphers_treeWidget->currentItem();
    if(item) {
        int  row = this->ui->Ciphers_treeWidget->currentIndex().row();

        if (item && row < (this->ui->Ciphers_treeWidget->topLevelItemCount() -1))
        {
                this->ui->Ciphers_treeWidget->takeTopLevelItem(row);
                this->ui->Ciphers_treeWidget->insertTopLevelItem(row + 1,item);
                this->ui->Ciphers_treeWidget->setCurrentItem(item);


                QList<QSslCipher>   ciphers;
                register size_t n = this->ui->Ciphers_treeWidget->topLevelItemCount();
                for(register unsigned int i = 0;i < n;++i) {
                    QTreeWidgetItem*    item = this->ui->Ciphers_treeWidget->topLevelItem(i);
                    if(item->checkState(0) == Qt::Checked) {
                        QSsl::SslProtocol   protocol;
                        if(item->text(2) == QLatin1String("TLSv1.2")) {
                            protocol = QSsl::TlsV1_2;
                        }
                        if(item->text(2) == QLatin1String("SSLv3")) {
                            protocol = QSsl::SslV3;
                        }

                        ciphers.append(QSslCipher(item->text(0),protocol));
                    }
                }

                QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
                sslConfig.setCiphers(ciphers);
                QSslConfiguration::setDefaultConfiguration(sslConfig);

        }
    }
}


void
encryption::on_Ciphers_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if(item) {
        QList<QSslCipher>   ciphers;

        register size_t n = this->ui->Ciphers_treeWidget->topLevelItemCount();
        for(register unsigned int i = 0;i < n;++i) {
            QTreeWidgetItem*    item = this->ui->Ciphers_treeWidget->topLevelItem(i);
            if(item->checkState(0) == Qt::Checked) {
                QSsl::SslProtocol   protocol;
                if(item->text(2) == QLatin1String("TLSv1.2")) {
                    protocol = QSsl::TlsV1_2;
                }
                if(item->text(2) == QLatin1String("SSLv3")) {
                    protocol = QSsl::SslV3;
                }

                ciphers.append(QSslCipher(item->text(0),protocol));
            }
        }

        qDebug() << "==========================================";
        qDebug() << ciphers;
        qDebug() << "==========================================";
        QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
        sslConfig.setCiphers(ciphers);
        QSslConfiguration::setDefaultConfiguration(sslConfig);

    }

}


void
encryption::on_CopyToMCB_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QTreeWidget*    treeWidget = qobject_cast<QTreeWidget*>(this->ui->BlockedMCB_stackedWidget->currentWidget());
    if(treeWidget) {
        QList<QTreeWidgetItem*> items = treeWidget->selectedItems();
        register size_t n = items.count();
        for(register unsigned int i = 0;i < n;++i) {
            QUrl    currentUrl = items.at(i)->text(0);
            QString text = currentUrl.host() + currentUrl.path();
            text.replace(QLatin1String("."),QLatin1String("\\."));
            text.replace(QLatin1String("?"),QLatin1String("\\?"));


            QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->DisableMCB_treeWidget);
            this->ui->DisableMCB_treeWidget->scrollToBottom();
            item->setCheckState(0,Qt::Checked);
            item->setFlags(item->flags() | Qt::ItemIsEditable);

            item->setText(0,text);

            this->ui->DisableMCB_treeWidget->editItem(item);
            this->ui->DisableMCB_treeWidget->addTopLevelItem(item);
        }
    }

}

void
encryption::on_RemoveDisabledMCBURL_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QTreeWidgetItem*    item = this->ui->DisableMCB_treeWidget->currentItem();
    if(item) {
        delete item;
    }

}

MainWindow*
encryption::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->mainWindow;
}

void encryption::on_AddDisabledMCBURL_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Browser*    browser = qobject_cast<Browser*>(this->getMainWindow()->currentTab());
    if(browser) {
        QUrl currentUrl = browser->getUrl();

        QString text = currentUrl.host() + currentUrl.path();
        text.replace(QLatin1String("."),QLatin1String("\\."));
        text.replace(QLatin1String("?"),QLatin1String("\\?"));

        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->DisableMCB_treeWidget);
        this->ui->DisableMCB_treeWidget->scrollToBottom();
        item->setCheckState(0,Qt::Checked);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        item->setText(0,text);

        this->ui->DisableMCB_treeWidget->editItem(item);
        this->ui->DisableMCB_treeWidget->addTopLevelItem(item);

    }

}

void encryption::on_HTTPSEnforcing_checkBox_toggled(bool checked)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    Q_ASSERT(this->mainWindow->generalTab != NULL);
    if(checked) {
        this->mainWindow->generalTab->ui->PreventGoogleTrackingRedirectLinks_checkBox->setChecked(true);
    }
}
