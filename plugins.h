/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef PLUGINS_H
#define PLUGINS_H

#include <QTreeWidgetItem>
#include <private/qwebplugindatabase_p.h>

using namespace WebCore;

namespace Ui {
class Plugins;
}

class Plugins : public QWidget
{
    Q_OBJECT
public:
    explicit Plugins(QWidget *parent = NULL);
    ~Plugins();
    void            loadSettings();
    void            saveSettings();
private slots:
    void                    on_Plugins_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void                    on_Plugins_treeWidget_itemChanged(QTreeWidgetItem *item, int column);
private:
    QString                 settingsFile;
    QWebPluginDatabase*     database;
    QList<QWebPluginInfo>   plugins;
    Ui::Plugins*            ui;
};

#endif // PLUGINS_H
