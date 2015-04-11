/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "cookieviewdelegate.h"


CookieViewDelegate::CookieViewDelegate(Cookie * c) :
    QStyledItemDelegate(c)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    _state =  QStyle::State_Enabled;
    _currentRow = -1;
}

bool CookieViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,const QModelIndex &index)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if(index.isValid()) {
        qDebug() << "ROW " << index.row();
        qDebug() << "COL " << index.column();

        switch(event->type()) {
            case    QEvent::QEvent::MouseButtonDblClick:
                qDebug() << "EVENT DOUBLECLICK";
            break;
            case    QEvent::MouseButtonPress:
                qDebug() << "EVENT MOUSEBUTTONPRESS";
            break;
            case    QEvent::MouseButtonRelease:
            {
                qDebug() << "EVENT MOUSEBUTTONRELEASE";
                if(index.column() == 0)
                    emit buttonClicked(index);
            }
            break;
            default:
                return QStyledItemDelegate::editorEvent(event,model,option,index);
        }
    }
    return QStyledItemDelegate::editorEvent(event,model,option,index);
}

void CookieViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if (!index.isValid())
        return;

    switch(index.column()) {
        case 0:
        {

         QStyleOptionButton button;
         button.features |= QStyleOptionButton::Flat;

         button.rect = option.rect;
         button.rect.setHeight(16);
         button.rect.setWidth(16+10);
         button.iconSize = QSize(16,16);

         if(index.data(Qt::DisplayRole) == QLatin1String("Blocked")) {
            button.icon = QIcon(QLatin1String(":/icons/16x16/dialog-no-2.png"));
            QTreeWidgetItem* item = qobject_cast<Cookie*>(parent())->cookieForRow(index.row());
            if(item)
                item->setToolTip(0,QLatin1String("Blocked"));
         }
         if(index.data(Qt::DisplayRole) == QLatin1String("Allowed")) {
            button.icon = QIcon(QLatin1String(":/icons/16x16/dialog-yes.png"));
            QTreeWidgetItem* item = qobject_cast<Cookie*>(parent())->cookieForRow(index.row());
            if(item)
                item->setToolTip(0,QLatin1String("Allowed"));
         }


         button.state = QStyle::State_Enabled;
         QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
        }
        break;
        default:
            QStyledItemDelegate::paint(painter, option, index);
    }
}

QWidget* CookieViewDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if (index.isValid())  {
        switch(index.column()) {
            case 0:
                return NULL;
            break;
        }
    }
        return  QStyledItemDelegate::createEditor(parent, option, index);
}
