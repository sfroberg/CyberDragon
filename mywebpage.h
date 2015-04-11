/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MYWEBPAGE_H
#define MYWEBPAGE_H

#include <QWebPage>

class MainWindow;

class MyWebPage : public QWebPage
{
    Q_OBJECT
public:
    explicit MyWebPage(MainWindow* m, QObject * parent = NULL);
    QString 	userAgentForUrl(const QUrl & url) const;
    bool        acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest & request, NavigationType type);
private:
    MainWindow* mainWindow;
};

#endif // MYWEBPAGE_H
