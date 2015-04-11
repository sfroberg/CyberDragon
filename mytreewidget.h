/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MYTREEWIDGET_H
#define MYTREEWIDGET_H

#include <QTreeWidget>
#include <QNetworkCookie>

class   MyTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    explicit    MyTreeWidget(QWidget* parent = NULL);
public slots:
    void        updateTrackersBlocked(const QStringList&);
    void        updateCookieList(QStringList data);
    void        updateAllowedCookieList(QList<QNetworkCookie> cookies);
    void        customSortByColumn(int column);
    void        halfEncryptedSlotFound(const QString& url);
};

#endif // MYTREEWIDGET_H
