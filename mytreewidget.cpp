/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "mytreewidget.h"
#include <QHeaderView>

MyTreeWidget::MyTreeWidget(QWidget* parent):
    QTreeWidget(parent)
{
}

void    MyTreeWidget::updateTrackersBlocked(const QStringList& text)
{
    this->addTopLevelItem(new QTreeWidgetItem(text));
}

void
MyTreeWidget::updateAllowedCookieList(QList<QNetworkCookie> cookies)
{
    this->clear();
    register size_t  n = cookies.count();
    for(register unsigned int i = 0;i < n;++i) {
        QTreeWidgetItem*    item = new QTreeWidgetItem(this);
        item->setText(0,cookies.at(i).domain());
        item->setText(1,cookies.at(i).path());
        item->setText(2,cookies.at(i).name() + QLatin1Literal("=") + cookies.at(i).value());
    }
}

void
MyTreeWidget::updateCookieList(QStringList data)
{

    QTreeWidgetItem*    item = new QTreeWidgetItem(this);

    item->setText(0,data.at(0));
    if(data.at(1) == QLatin1String("Allowed")) {
        item->setIcon(1,QIcon(QLatin1String(":/icons/16x16/dialog-yes.png")));
        item->setToolTip(1,data.at(1));
    }
    if(data.at(1) == QLatin1String("Blocked")) {
        item->setIcon(1,QIcon(QLatin1String(":/icons/16x16/dialog-no-2.png")));
        QString text = data.at(1) + QLatin1String(": ") + data.at(8);
        item->setToolTip(1,text);
    }

    item->setText(1,data.at(1));
    item->setText(2,data.at(2));
    item->setText(3,data.at(3));
    item->setText(4,data.at(4));
    item->setText(5,data.at(5));
    item->setText(6,data.at(6));
    item->setText(7,data.at(7));
}

void MyTreeWidget::customSortByColumn(int column)
{
// here you can get the order
    Qt::SortOrder order = this->header()->sortIndicatorOrder();

// and sort the items
    this->sortItems(column, order);

// to get more control over actual sorting of items,
// reimplement QTreeWidgetItem::operator<()
}

void    MyTreeWidget::halfEncryptedSlotFound(const QString& url)
{
    QTreeWidgetItem*    item = new QTreeWidgetItem(this);
    item->setText(0,url);
}
