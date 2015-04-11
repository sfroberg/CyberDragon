/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef COOKIEVIEWDELEGATE_H
#define COOKIEVIEWDELEGATE_H

#include <QStyledItemDelegate>
#include <QDebug>

#include "cookie.h"
#include "ui_cookie.h"

/******************************************************/
class CookieViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    CookieViewDelegate(Cookie * c);
    bool        editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,const QModelIndex &index);
    void        paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QWidget*    createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
signals:
    void        buttonClicked(const QModelIndex &index);
private:
    QStyle::State   _state;
    int             _currentRow;
};

#endif // COOKIEVIEWDELEGATE_H
