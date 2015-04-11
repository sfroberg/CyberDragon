/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef TRACKERBLOCKER_H
#define TRACKERBLOCKER_H

#include <QTreeWidgetItem>

namespace Ui {
class trackerblocker;
}

class MainWindow;

class trackerblocker : public QWidget
{
    Q_OBJECT

public:
    static  int     TRACKER_BLOCKER_LIST_COUNT;
    static  int     TRACKER_BLOCKER_ADDED_COUNT;
    static  int     TRACKER_BLOCKER_REMOVED_COUNT;

    explicit trackerblocker(MainWindow* m, QWidget *parent = 0);
    ~trackerblocker();
    inline  void    loadSettings();
    inline  void    loadTrackerBlockerRules();
    inline  void    saveSettings();

    MainWindow* getMainWindow();
signals:
    void    trackerBlockerDisabled(bool);
private slots:
    void on_BlockTrackers_checkBox_toggled(bool checked);

    void on_TrackerBlockerExportRules_pushButton_clicked();

    void on_TrackerBlockerSave_pushButton_clicked();

    void    addTrackerBlocker();
    void    removeTrackerBlocker();
    void    on_TrackerBlockerImportRules_pushButton_clicked();
    void    trackerBlockerChanged( QTreeWidgetItem * item, int column);
    void on_TrackerBlockerFile_pushButton_clicked();

private:
    QString     settingsFile;
    friend class Browser;
    friend class MainWindow;
    friend class MyNetworkAccessManager;
    MainWindow* mainWindow;
    Ui::trackerblocker *ui;
};

#endif // TRACKERBLOCKER_H
