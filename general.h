/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef GENERAL_H
#define GENERAL_H

#include <QButtonGroup>
#include <QTreeWidgetItem>

#include "mainwindow.h"

namespace Ui {
class general;
}

class general : public QWidget
{
    Q_OBJECT
public:
    explicit general(MainWindow* m, QWidget *parent = NULL);
    ~general();
    void            loadSettings();
    void            loadUserAgents();
    void            saveSettings();
public slots:
    void            buttonClicked(int id);
    void            randomUserAgent();
private slots:
    void            on_UserAgent_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void            on_UserAgentManually_radioButton_toggled(bool checked);
    void            on_UserAgentRandomly_radioButton_toggled(bool checked);
    void            on_SetHomePage_pushButton_clicked();
private:
    friend class    Browser;
    friend class    MainWindow;
    friend class    MyNetworkAccessManager;
    friend class    MyWebPage;
    friend class    encryption;
    MainWindow*     mainWindow;
    QButtonGroup*   settings;
    QString         settingsFile;
    QButtonGroup*   httpReferer;
    Ui::general*    ui;
};

namespace HTTPReferer {
    enum {
        DoNotTouch,
        Remove,
        SameAsCurrentUrl,
        CustomHTTPReferer
    };
}


#endif // GENERAL_H
