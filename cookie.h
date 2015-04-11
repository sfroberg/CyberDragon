/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef COOKIE_H
#define COOKIE_H

#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace Ui {
class cookie;
}

class Cookie : public QWidget
{
    Q_OBJECT
public:
    explicit Cookie(QWidget *parent = 0);
    ~Cookie();
    QTreeWidgetItem*    cookieForRow(int index);
    void                loadSettings();
    void                saveSettings();
signals:
    void                ClearAllCookies();
private slots:
    void                on_SpoofGooglePREFCookie_checkBox_toggled(bool checked);
    void                on_ClearCookieList_pushButton_clicked();
    void                on_CopyToCustomCookieRules_pushButton_clicked();
    void                addCookieRule();
    void                moveCookieRuleDown();
    void                moveCookieRuleUp();
    void                removeCustomCookieRule();
    void                allowDenyCookie(const QModelIndex &index);
    void                on_ClearAllowedCookies_pushButton_clicked();
private:
    friend class        Browser;
    friend class        MainWindow;
    friend class        MyCookieJar;
    friend class        MyNetworkAccessManager;
    QString             settingsFile;
    Ui::cookie*         ui;
};

#endif // COOKIE_H
