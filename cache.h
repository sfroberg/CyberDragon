/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef CACHE_H
#define CACHE_H

#include <QWidget>
#include <QButtonGroup>

namespace Ui {
class cache;
}

class   MainWindow;

class Cache : public QWidget
{
    Q_OBJECT
public:
    explicit Cache(MainWindow *parent = 0);
    ~Cache();
    QString     getCache();
    void        setMainWindow(MainWindow* m);
    MainWindow* getMainWindow();
    void        loadSettings();
    void        saveSettings();
public slots:
    void            cacheButtonClicked(int);
    void            update();
private slots:
    void            on_setCache_pushButton_clicked();
    void            on_cacheDir_pushButton_clicked();
    void            on_clearCache_pushButton_clicked();
private:
    friend class    MainWindow;
    friend class    Browser;
    friend class    MyNetworkAccessManager;
    MainWindow*     mainWindow;
    QButtonGroup*   cacheSettings;
    QString         settingsFile;
    Ui::cache*      ui;
};

namespace DiskCache {
    enum {
        NoCache,
        CacheEnabled,
        OfflineCache
    };
}

#endif // CACHE_H
