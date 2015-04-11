/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "trackerblocker.h"
#include "ui_trackerblocker.h"

#include "notification.h"
#include "ui_notification.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QFile>
#include <QDebug>
#include <QSettings>

#include <QFileDialog>
#include <QDir>

int     trackerblocker::TRACKER_BLOCKER_LIST_COUNT = 0;
int     trackerblocker::TRACKER_BLOCKER_ADDED_COUNT = 0;
int     trackerblocker::TRACKER_BLOCKER_REMOVED_COUNT = 0;


/* TODO: Custom error page 404 for poor souls who actually visited the blocked tracker ... */
/* TODO2: This trackerblocker class uses QTreeWidget and loops throught it for each URL, and setting &testing regular expression
 * pattern match with each iteration. Would QTreeView be more faster???? Some other way??? */

trackerblocker::trackerblocker(MainWindow* m, QWidget *parent) :
    QWidget(parent),settingsFile(QLatin1String("config.ini")),mainWindow(m),
    ui(new Ui::trackerblocker)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);

    this->loadSettings();
    this->loadTrackerBlockerRules();


    this->ui->TrackerBlockerRules_treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    QAction*    action_addTrackerBlocker = new QAction(tr("Add tracker blocker rule ..."),this->ui->TrackerBlockerRules_treeWidget);
    connect(action_addTrackerBlocker,SIGNAL(triggered()),this,SLOT(addTrackerBlocker()));
    this->ui->TrackerBlockerRules_treeWidget->addAction(action_addTrackerBlocker);

    QAction*    action_removeTrackerBlocker = new QAction(tr("Remove tracker blocker rule ..."),this->ui->TrackerBlockerRules_treeWidget);
    connect(action_removeTrackerBlocker,SIGNAL(triggered()),this,SLOT(removeTrackerBlocker()));
    this->ui->TrackerBlockerRules_treeWidget->addAction(action_removeTrackerBlocker);


}

trackerblocker::~trackerblocker()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->saveSettings();
    delete ui;
}


void
trackerblocker::addTrackerBlocker()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    trackerblocker::TRACKER_BLOCKER_ADDED_COUNT++;

    this->ui->TrackerBlockerRules_treeWidget->scrollToTop();
    QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->TrackerBlockerRules_treeWidget);
    item->setCheckState(0,Qt::Checked);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    this->ui->TrackerBlockerRules_treeWidget->editItem(item);

}

void
trackerblocker::removeTrackerBlocker()
{
    QList<QTreeWidgetItem*> list = this->ui->TrackerBlockerRules_treeWidget->selectedItems();
    foreach(QTreeWidgetItem* item,list) {
        trackerblocker::TRACKER_BLOCKER_REMOVED_COUNT++;
        delete item;
    }

}


MainWindow*
trackerblocker::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->mainWindow;
}


void
trackerblocker::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("tracker_blocker"));
    this->ui->BlockTrackers_checkBox->setChecked(settings.value(QLatin1String("block_trackers")).toBool());
    this->on_BlockTrackers_checkBox_toggled(settings.value(QLatin1String("block_trackers")).toBool());
    this->ui->TrackerBlockerFile_lineEdit->setText(settings.value(QLatin1String("tracker_blocker_file")).toString());
    settings.endGroup();
}


void
trackerblocker::loadTrackerBlockerRules()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QFile   file(this->ui->TrackerBlockerFile_lineEdit->text());
    if(file.open(QIODevice::ReadOnly | QIODevice::Text)) {

        int n = 0;

        QSettings settings(this->settingsFile, QSettings::IniFormat);

        disconnect(this->ui->TrackerBlockerRules_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(trackerBlockerChanged(QTreeWidgetItem*,int)));
        this->ui->TrackerBlockerRules_treeWidget->clear();

        while(!file.atEnd()) {
            QString line = file.readLine().trimmed();
            if(!line.isEmpty()) {
                QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->TrackerBlockerRules_treeWidget);
                item->setText(0,line);
                item->setFlags(item->flags() | Qt::ItemIsEditable);

                item->setCheckState(0,Qt::Checked);

                int size = settings.beginReadArray(QLatin1String("rule"));
                for (int i = 0; i < size; ++i) {
                    settings.setArrayIndex(i);
                        if(settings.value(QLatin1String("rule")).toString() == item->text(0)) {
                            item->setCheckState(0,Qt::Unchecked);
                        }
                }
                settings.endArray();
                ++n;
            }
        }
        this->ui->NumberTrackerBlockerRules_groupBox->setTitle("Number of Tracker Blocker Rules Loaded: " + QString::number(n));
        file.close();
        connect(this->ui->TrackerBlockerRules_treeWidget,SIGNAL(itemChanged(QTreeWidgetItem*,int)),this,SLOT(trackerBlockerChanged(QTreeWidgetItem*,int)));

    }
    this->ui->TrackerBlockerRules_treeWidget->sortByColumn(0,Qt::AscendingOrder);

}


void
trackerblocker::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("tracker_blocker"));
    settings.setValue(QLatin1String("block_trackers"),this->ui->BlockTrackers_checkBox->isChecked());
    settings.setValue(QLatin1String("tracker_blocker_file"),this->ui->TrackerBlockerFile_lineEdit->text());
    settings.endGroup();

    settings.remove(QLatin1String("rule"));
    settings.beginWriteArray(QLatin1String("rule"));
    int n = 0;
    for (int i = 0; i < this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount(); ++i) {
        if(this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->checkState(0) == Qt::Unchecked) {
            settings.setArrayIndex(n);
            settings.setValue(QLatin1String("rule"),this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->text(0));
            ++n;
        }
    }
    settings.endArray();


}

void trackerblocker::on_BlockTrackers_checkBox_toggled(bool checked)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    emit trackerBlockerDisabled(checked);

    this->ui->TrackerBlockerRules_treeWidget->setEnabled(checked);
    this->ui->TrackerBlockerExportRules_pushButton->setEnabled(checked);
    this->ui->TrackerBlockerFile_lineEdit->setEnabled(checked);
    this->ui->TrackerBlockerFile_pushButton->setEnabled(checked);
    this->ui->TrackerBlockerImportRules_pushButton->setEnabled(checked);
    this->ui->TrackerBlockerSave_pushButton->setEnabled(checked);
}

void trackerblocker::on_TrackerBlockerExportRules_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QString filename = QFileDialog::getSaveFileName(this, tr("Save File"),
                               QDir::currentPath(),
                                tr("Text (*.txt);; Any (*.*)"));


    if(!filename.isEmpty() && !filename.isNull()) {
        QFile   file(filename);
        if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {



            QList<QTreeWidgetItem*> items = this->ui->TrackerBlockerRules_treeWidget->selectedItems();

            /* If selected, then export only those. Otherwise export all */
            if(items.count() > 0) {
                register size_t n = items.count();
                for(register unsigned int i = 0;i < n;++i) {
                    if(items.at(i)->text(0).isEmpty())
                        continue;
                    file.write(items.at(i)->text(0).toLatin1() + "\r\n");
                }
                this->ui->TrackerBlockerRules_treeWidget->clearSelection();


                Q_ASSERT(this->mainWindow->notificationTab != NULL);
                if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked()) {
                    Notify* notify = new Notify(QLatin1String("Exported Tracker Blocker Rule(s)"),"Exported "+ QString::number(items.count()) + " tracker blocker rule(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                    notify->show();
                }

            } else {
                register size_t n = this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount();
                for(register unsigned int i = 0;i < n;++i) {
                    if(this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->text(0).isEmpty())
                        continue;
                    file.write(this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->text(0).toLatin1() + "\r\n");
                }
                this->ui->TrackerBlockerRules_treeWidget->clearSelection();

                Q_ASSERT(this->mainWindow->notificationTab != NULL);
                if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked()) {
                    Notify* notify = new Notify(QLatin1String("Exported Tracker Blocker Rule(s)"),"Exported "+ QString::number(n) + " tracker blocker rule(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                    notify->show();
                }


            }
        }
        file.close();
    }

}

void trackerblocker::on_TrackerBlockerSave_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QFile   file(this->ui->TrackerBlockerFile_lineEdit->text());
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        register size_t n = this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount();
        for(register unsigned int i = 0;i < n;++i) {
            if(this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->text(0).isEmpty())
                continue;
            file.write(this->ui->TrackerBlockerRules_treeWidget->topLevelItem(i)->text(0).toLatin1() + "\r\n");
        }
        file.close();

        this->ui->NumberTrackerBlockerRules_groupBox->setTitle("Number of Tracker Blocking Rules Loaded: " + QString("%1").arg(this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount()));

        Q_ASSERT(this->mainWindow->notificationTab != NULL);
        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyTrackerBlockerAdded_checkBox->isChecked()) {
            if(this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount() > trackerblocker::TRACKER_BLOCKER_LIST_COUNT) {
                Notify* notify = new Notify(QLatin1String("Added New Tracker Blocker Rule(s)"),"Added "+ QString::number((this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount() - trackerblocker::TRACKER_BLOCKER_LIST_COUNT)) + " new tracker blocker rule(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
            }
        }

        if(this->mainWindow->notificationTab->ui->notifications_checkBox->isChecked() && this->mainWindow->notificationTab->ui->notifyTrackerBlockerRemoved_checkBox->isChecked()) {
            if(this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount() < trackerblocker::TRACKER_BLOCKER_LIST_COUNT) {
                Notify* notify = new Notify(QLatin1String("Removed Tracker Blocker Rule(s)"),"Removed "+ QString::number((trackerblocker::TRACKER_BLOCKER_LIST_COUNT - this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount())) + " tracker blocker rule(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                notify->show();
            }
        }
    }

    trackerblocker::TRACKER_BLOCKER_ADDED_COUNT = 0;
    trackerblocker::TRACKER_BLOCKER_REMOVED_COUNT = 0;

}

/* FIXME: Test more! */
void
trackerblocker::on_TrackerBlockerImportRules_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                   QDir::currentPath(),
                                   tr("Text (*.txt);; Any (*.*)"));

    if(!filename.isEmpty() && !filename.isNull()) {
        QFile   file(filename);
        if(file.open(QIODevice::ReadOnly)) {

            trackerblocker::TRACKER_BLOCKER_LIST_COUNT = this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount();
            int n = 0;

            while(!file.atEnd()) {
                QString line = file.readLine().trimmed();
                if(!line.isEmpty()) {
                    QTreeWidgetItem* item = new QTreeWidgetItem(this->ui->TrackerBlockerRules_treeWidget);
                    item->setCheckState(0,Qt::Checked);
                    item->setText(0,line);
                    n++;
                }
            }
            if(this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount() > trackerblocker::TRACKER_BLOCKER_LIST_COUNT)
            {
                Q_ASSERT(this->mainWindow->notificationTab != NULL);

                    Notify* notify = new Notify(QLatin1String("Imported Tracker Blocker Rule(s)"),"Imported "+ QString::number((this->ui->TrackerBlockerRules_treeWidget->topLevelItemCount() - trackerblocker::TRACKER_BLOCKER_LIST_COUNT)) + " new tracker blocker rule(s) ...",this->mainWindow->notificationTab->ui->notifyTimeout_spinBox->value(),this);
                    notify->show();
            }
        }
    }
}



void
trackerblocker::trackerBlockerChanged( QTreeWidgetItem * item, int column)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    /* tracker blocker rule list has changed, remove duplicate rules. */
    if(item) {
        QList<QTreeWidgetItem*> list = this->ui->TrackerBlockerRules_treeWidget->findItems(item->text(0),Qt::MatchExactly);
        if(list.count() > 1) {
            register size_t n = list.count();
            for(register unsigned int i = 1;i < n;++i) {
                delete list.at(i);
            }
        }
    }
}

void
trackerblocker::on_TrackerBlockerFile_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QString filename = QFileDialog::getOpenFileName(this, tr("Open File"),
                                   QDir::currentPath(),
                                   tr("Text (*.txt);; Any (*.*)"));

    if(!filename.isEmpty() && !filename.isNull()) {

        this->ui->TrackerBlockerFile_lineEdit->setText(filename);
        this->loadTrackerBlockerRules();


    }

}
