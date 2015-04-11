/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <QWidget>
#include <QTreeWidgetItem>

namespace Ui {
class encryption;
}

class   MainWindow;

class encryption : public QWidget
{
    Q_OBJECT
public:
    explicit encryption(MainWindow* m, QWidget *parent = 0);
    ~encryption();
    void            loadSettings();
    void            saveSettings();
    MainWindow*     getMainWindow();
private slots:
    void            on_MoveCipherUp_pushButton_clicked();
    void            on_MoveCipherDown_pushButton_clicked();
    void            on_Ciphers_treeWidget_itemChanged(QTreeWidgetItem *item, int column);
    void            on_CopyToMCB_pushButton_clicked();
    void            on_RemoveDisabledMCBURL_pushButton_clicked();
    void            on_AddDisabledMCBURL_pushButton_clicked();
    void            on_HTTPSEnforcing_checkBox_toggled(bool checked);
private:
    friend class    Browser;
    friend class    MainWindow;
    friend class    MyNetworkAccessManager;
    MainWindow*     mainWindow;
    QString         settingsFile;
    Ui::encryption* ui;
};

#endif // ENCRYPTION_H
