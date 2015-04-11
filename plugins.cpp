/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "plugins.h"
#include "ui_plugins.h"

#include <QSettings>
#include <QDebug>

#include <QWebSettings>

/* This uses the Nokia QWebPluginDatabase API. The only thing that I could find for handling browser plugins in Qt.

 * Unfortunately the API is private by default and Digia, the current manager of Qt, has not cared of making it public.
 * (to tell the truth, Digia does not care anything about improving the Qt core functionality: widgets, networking etc..
 * All they care are just shiny, eye candy stuff for mobile and anything to do with mobile, like quick & qml)
 *
 * So as an ugly hack, I have shipped private/qwebplugindatabe_p.h private header with CyberDragon source and changed the
 * API from private to public (just move the QWebPluginDatabase constructor & destructor from private to public)
 *
 * Of course, this means that the damn file must be keeped in sync with each new release of Qt to prevent compilatin errors.

  *Sight!*
 *
 */

class NPAPI
{
public:
    static QWebPluginDatabase *pluginDatabase()
    {
        static QWebPluginDatabase* database = 0;
            if (!database)
                database = new QWebPluginDatabase();
            return database;
    }
};


Plugins::Plugins(QWidget *parent) :
    QWidget(parent),settingsFile(QLatin1String("config.ini")),
    ui(new Ui::Plugins)
{
    ui->setupUi(this);

    this->database = NPAPI::pluginDatabase();
    if(this->database) {
        this->plugins = database->plugins();

        this->loadSettings();
    }
}

Plugins::~Plugins()
{
    this->saveSettings();
    delete ui;
}

void
Plugins::loadSettings()
{
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("plugins"));

    int size = settings.beginReadArray(QLatin1String("plugin"));

    if(size > 0) {
        for (int i = 0; i < size; ++i) {
            settings.setArrayIndex(i);
            foreach(QWebPluginInfo info,this->plugins) {
                if(info.name() == settings.value("name").toString()) {
                    info.setEnabled(settings.value("enabled").toBool());
                    break;
                }
            }
        }
    } else {
    }

    foreach(QWebPluginInfo info,this->plugins) {
        qDebug() << info.isEnabled();
        qDebug() << info.name();
        qDebug() << info.description();
        qDebug() << info.path();

        QStringList text;
        text << info.name() << info.description() << info.path();
        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->Plugins_treeWidget,text);
        item->setCheckState(0,(info.isEnabled() ? Qt::Checked : Qt::Unchecked) );


        foreach(QWebPluginInfo::MimeType type,info.mimeTypes()) {
            qDebug() << type.name << "\t" << type.fileExtensions << "\t" << type.description;
        }
    }
    this->ui->Plugins_treeWidget->resizeColumnToContents(0);
    this->ui->Plugins_treeWidget->resizeColumnToContents(2);
    this->ui->Plugins_treeWidget->sortByColumn(0,Qt::AscendingOrder);

    settings.endArray();
    settings.endGroup();
}

void
Plugins::saveSettings()
{
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("plugins"));
    settings.remove(QLatin1String("plugin")); // remove any previous plugin stuff ...
    settings.beginWriteArray(QLatin1String("plugin"));

    for (int i = 0; i < this->ui->Plugins_treeWidget->topLevelItemCount(); ++i) {
        settings.setArrayIndex(i);
        QTreeWidgetItem* item = this->ui->Plugins_treeWidget->topLevelItem(i);
        settings.setValue(QLatin1String("name"),item->text(0));
        settings.setValue(QLatin1String("enabled"),(item->checkState(0) == Qt::Checked));
    }
    settings.endArray();
    settings.endGroup();
}


void Plugins::on_Plugins_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
/* Nothing for now ... */
}

void Plugins::on_Plugins_treeWidget_itemChanged(QTreeWidgetItem *item, int column)
{
    if(item) {
        foreach(QWebPluginInfo info,this->plugins) {
            if(info.name() == item->text(0)) {
                info.setEnabled((item->checkState(0) == Qt::Checked));
                break;
            }
        }
    }
}
