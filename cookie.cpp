/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "cookie.h"
#include "ui_cookie.h"

#include <QDebug>
#include <QDateTime>
#include <QSettings>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "mytreewidget.h"
#include "cookieviewdelegate.h"

inline uint qHash(const QStringList& key, uint seed = 0)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return qHash(key.join(QLatin1String(",")),seed);
}

Cookie::Cookie(QWidget *parent) :
    QWidget(parent),
    settingsFile(QLatin1String("config.ini")),ui(new Ui::cookie)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);
    this->loadSettings();

    this->ui->CustomCookieRules_treeWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    QAction*    action_addCookieCustomCookieRule = new QAction(tr("Add custom cookie rule ..."),this->ui->CustomCookieRules_treeWidget);
    connect(action_addCookieCustomCookieRule,SIGNAL(triggered()),this,SLOT(addCookieRule()));
    this->ui->CustomCookieRules_treeWidget->addAction(action_addCookieCustomCookieRule);
    connect(this->ui->AddCustomCookieRule_pushButton,SIGNAL(clicked()),this,SLOT(addCookieRule()));

    QAction*    action_removeCustomCookieRule = new QAction(tr("Remove custom cookie rule ..."),this->ui->CustomCookieRules_treeWidget);
    connect(action_removeCustomCookieRule,SIGNAL(triggered()),this,SLOT(removeCustomCookieRule()));
    this->ui->CustomCookieRules_treeWidget->addAction(action_removeCustomCookieRule);
    connect(this->ui->RemoveCustomCookieRule_pushButton,SIGNAL(clicked()),this,SLOT(removeCustomCookieRule()));

    QAction*    action_moveCustomCookieRuleUp = new QAction(tr("Move custom cookie rule up ..."),this->ui->CustomCookieRules_treeWidget);
    connect(action_moveCustomCookieRuleUp,SIGNAL(triggered()),this,SLOT(moveCookieRuleUp()));
    this->ui->CustomCookieRules_treeWidget->addAction(action_moveCustomCookieRuleUp);
    connect(this->ui->MoveUpCustomCookieRule_pushButton,SIGNAL(clicked()),this,SLOT(moveCookieRuleUp()));

    QAction*    action_moveCustomCookieRuleDown = new QAction(tr("Move custom cookie rule down ..."),this->ui->CustomCookieRules_treeWidget);
    connect(action_moveCustomCookieRuleDown,SIGNAL(triggered()),this,SLOT(moveCookieRuleDown()));
    this->ui->CustomCookieRules_treeWidget->addAction(action_moveCustomCookieRuleDown);
    connect(this->ui->MoveDownCustomCookieRule_pushButton,SIGNAL(clicked()),this,SLOT(moveCookieRuleDown()));

    CookieViewDelegate*    CookieDelegate = new CookieViewDelegate(this);
    this->ui->CustomCookieRules_treeWidget->setItemDelegate(CookieDelegate);
    connect(CookieDelegate,SIGNAL(buttonClicked(QModelIndex)),this,SLOT(allowDenyCookie(QModelIndex)));
    this->ui->CustomCookieRules_treeWidget->header()->setSortIndicatorShown(true);

    this->ui->CustomCookieRules_treeWidget->header()->setSectionsClickable(true);
    connect(this->ui->CustomCookieRules_treeWidget->header(),SIGNAL(sectionClicked(int)),this->ui->CustomCookieRules_treeWidget,SLOT(customSortByColumn(int)));
}

Cookie::~Cookie()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->saveSettings();
    delete ui;
}

void
Cookie::on_SpoofGooglePREFCookie_checkBox_toggled(bool checked)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->ui->ManuallySpoofGooglePREFCookie_radioButton->setEnabled(checked);
    this->ui->RandomSpoofGooglePREFCookie_radioButton->setEnabled(checked);
    this->ui->SpoofGooglePREFCookie_comboBox->setEnabled(checked);
    this->ui->SpoofGooglePREFCookie_lineEdit->setEnabled(checked);
    this->ui->SpoofGooglePREFCookie_label->setEnabled(checked);
}

QTreeWidgetItem*
Cookie::cookieForRow(int index)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return this->ui->CustomCookieRules_treeWidget->topLevelItem(index);
}

void
Cookie::addCookieRule()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->CustomCookieRules_treeWidget);
    this->ui->CustomCookieRules_treeWidget->scrollToBottom();
    item->setText(0,QLatin1String("Allowed"));
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    this->ui->CustomCookieRules_treeWidget->editItem(item,1);
}

void
Cookie::moveCookieRuleDown()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QTreeWidgetItem*    item = this->ui->CustomCookieRules_treeWidget->currentItem();
    if(item) {
        int                 row = this->ui->CustomCookieRules_treeWidget->currentIndex().row();

        if (item && row < (this->ui->CustomCookieRules_treeWidget->topLevelItemCount() -1))
        {
                this->ui->CustomCookieRules_treeWidget->takeTopLevelItem(row);
                this->ui->CustomCookieRules_treeWidget->insertTopLevelItem(row + 1,item);
                this->ui->CustomCookieRules_treeWidget->setCurrentItem(item);
        }
    }
}

void
Cookie::moveCookieRuleUp()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QTreeWidgetItem*    item = this->ui->CustomCookieRules_treeWidget->currentItem();
    if(item) {
        int                 row = this->ui->CustomCookieRules_treeWidget->currentIndex().row();

        if (item && row > 0)
        {
                this->ui->CustomCookieRules_treeWidget->takeTopLevelItem(row);
                this->ui->CustomCookieRules_treeWidget->insertTopLevelItem(row - 1,item);
                this->ui->CustomCookieRules_treeWidget->setCurrentItem(item);
        }
    }
}

void
Cookie::removeCustomCookieRule()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QList<QTreeWidgetItem*> items = this->ui->CustomCookieRules_treeWidget->selectedItems();
    register size_t n = items.count();
    for(register unsigned int i = 0;i < n;++i) {
        delete items.at(i);
    }
}

void
Cookie::allowDenyCookie(const QModelIndex &index)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if(index.isValid()) {
        QTreeWidgetItem* item = this->ui->CustomCookieRules_treeWidget->topLevelItem(index.row());

        if(index.data() == QLatin1String("Allowed")) {
            item->setText(index.column(),QLatin1String("Blocked"));
            qDebug() << "Cookie was allowed, changing to blocked";
        } else {
            item->setText(index.column(),QLatin1String("Allowed"));
            qDebug() << "Cookie was blocked, changing to allowed";
        }
        this->ui->CustomCookieRules_treeWidget->viewport()->repaint();
    }
}

void
Cookie::on_ClearCookieList_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    MyTreeWidget*   w = qobject_cast<MyTreeWidget*>(this->ui->CookieList_stackedWidget->currentWidget());

    if(w && w->topLevelItemCount() > 0)
        w->clear();
}

void Cookie::on_CopyToCustomCookieRules_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    MyTreeWidget*   widget = qobject_cast<MyTreeWidget*>(this->ui->CookieList_stackedWidget->currentWidget());
    if(widget) {
        if(widget->topLevelItemCount() > 0) {
            QList<QTreeWidgetItem*> items = widget->selectedItems();
            /* Copy only selected cookies to custom cookie rule.
             * Otherwise, copy all */
            if(items.count() > 0){

                /* First, remove duplicates from source */
                QSet<QStringList>   cookiesFrom;
                register size_t n = items.count();
                for(register unsigned int i = 0;i < n;++i) {
                    QStringList data;
                    data << items.at(i)->text(1) << items.at(i)->text(2) << items.at(i)->text(3) << items.at(i)->text(4).replace(QRegularExpression(QLatin1String("(^.[^=]*=)(.*)")),QLatin1String("\\1.*"));
                    cookiesFrom.insert(data);
                }
                /* Then make a target to compare to. Also remove duplicates althought there
                   should be any. (Not possible for user to enter duplicate) */

                QSet<QStringList>  cookiesTo;

                register size_t n2 = this->ui->CustomCookieRules_treeWidget->topLevelItemCount();
                for(register unsigned int i = 0;i < n2;++i) {
                    QStringList data;
                    QTreeWidgetItem*    item = this->ui->CustomCookieRules_treeWidget->topLevelItem(i);
                    data << item->text(0) << item->text(1) << item->text(2) << item->text(3).replace(QRegularExpression(QLatin1String("(^.[^=]*=)(.*)")),QLatin1String("\\1.*"));
                    cookiesTo.insert(data);
                }

                /* Now, iterate throught cookiesFrom set and any item that is not
                 * contained in cookiesTo set is inserted to CustomCookie_treeWidget */
                foreach(const QStringList& data,cookiesFrom) {
                    if(!cookiesTo.contains(data)) {

                        QTreeWidgetItem* item = new QTreeWidgetItem(this->ui->CustomCookieRules_treeWidget);
                        item->setText(0,data.at(0));  // Action
                        item->setToolTip(0,data.at(0));

                        QString domain = data.at(1);
                        domain.replace(QLatin1String("."),QLatin1String("\\."));
                        item->setText(1,domain);  // Domain
                        item->setText(2,data.at(2));  // Path
                        item->setText(3,data.at(3));  // Name-Value
                        item->setFlags(item->flags() | Qt::ItemIsEditable);
                        this->ui->CustomCookieRules_treeWidget->addTopLevelItem(item);
                        this->ui->CustomCookieRules_treeWidget->resizeColumnToContents(3);
                    }
                }

            } else {

                /* First, remove duplicates from source */
                QSet<QStringList>   cookiesFrom;
                register size_t n = widget->topLevelItemCount();
                for(register unsigned int i = 0;i < n;++i) {
                    QTreeWidgetItem* item = widget->topLevelItem(i);
                    QStringList data;
                    data << item->text(1) << item->text(2) << item->text(3) << item->text(4);
                    cookiesFrom.insert(data);
                }

                /* Then make a target to compare to. Also remove duplicates althought there
                   should be any. (Not possible for user to enter duplicate) */
                QSet<QStringList>  cookiesTo;
                n = this->ui->CustomCookieRules_treeWidget->topLevelItemCount();
                for(register unsigned int i = 0;i < n;++i) {
                    QStringList data;
                    QTreeWidgetItem*    item = this->ui->CustomCookieRules_treeWidget->topLevelItem(i);
                    data << item->text(0) << item->text(1) << item->text(2) << item->text(3);
                    cookiesTo.insert(data);
                }
                /* Now, iterate throught cookiesFrom set and any item that is not
                 * contained in cookiesTo set is inserted to CustomCookie_treeWidget */
                foreach(const QStringList& data,cookiesFrom) {
                    if(!cookiesTo.contains(data)) {

                        QTreeWidgetItem* item = new QTreeWidgetItem(this->ui->CustomCookieRules_treeWidget);
                        item->setText(0,data.at(0));  // Action
                        item->setToolTip(0,data.at(0));

                        QString domain = data.at(1);
                        domain.replace(QLatin1String("."),QLatin1String("\\."));
                        item->setText(1,domain);  // Domain
                        item->setText(2,data.at(2));  // Path
                        item->setText(3,data.at(3));  // Name-Value
                        item->setFlags(item->flags() | Qt::ItemIsEditable);
                        this->ui->CustomCookieRules_treeWidget->addTopLevelItem(item);
                        this->ui->CustomCookieRules_treeWidget->resizeColumnToContents(3);
                    }
                }
            }
            widget->clearSelection();
        }
    }

}

void
Cookie::on_ClearAllowedCookies_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    MyTreeWidget*   w = qobject_cast<MyTreeWidget*>(this->ui->AllowedCookies_stackedWidget->currentWidget());
    if(w) {
        w->clear();
        emit ClearAllCookies();
    }
}


void
Cookie::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("global_cookies"));
    this->ui->Allow3rdPartyCookies_checkBox->setChecked(settings.value(QLatin1String("3rdparty")).toBool());

    this->ui->SessionCookie_checkBox->setChecked(settings.value(QLatin1String("session")).toBool());
    this->ui->HttpOnlyCookie_checkBox->setChecked(settings.value(QLatin1String("httponly")).toBool());
    this->ui->SecureCookie_checkBox->setChecked(settings.value(QLatin1String("secure")).toBool());
    this->ui->ClearCookieListOnPageLoad_checkBox->setChecked(settings.value(QLatin1String("clear_cookie_list_on_each_page")).toBool());

    settings.endGroup();


    int size = settings.beginReadArray(QLatin1String("custom_cookies"));
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);

        QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->CustomCookieRules_treeWidget);
        item->setFlags(QFlag(settings.value(QLatin1String("flags")).toInt()));
        item->setText(0,settings.value(QLatin1String("action")).toString());
        item->setText(1,settings.value(QLatin1String("domain")).toString());
        item->setText(2,settings.value(QLatin1String("path")).toString());
        item->setText(3,settings.value(QLatin1String("name-value")).toString());
    }
    settings.endArray();
}

void
Cookie::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("global_cookies"));
    settings.setValue(QLatin1String("3rdparty"),this->ui->Allow3rdPartyCookies_checkBox->isChecked());

    settings.setValue(QLatin1String("session"),this->ui->SessionCookie_checkBox->isChecked());
    settings.setValue(QLatin1String("httponly"),this->ui->HttpOnlyCookie_checkBox->isChecked());
    settings.setValue(QLatin1String("secure"),this->ui->SecureCookie_checkBox->isChecked());
    settings.setValue(QLatin1String("clear_cookie_list_on_each_page"),this->ui->ClearCookieListOnPageLoad_checkBox->isChecked());
    settings.endGroup();

    settings.remove(QLatin1String("custom_cookies"));  /* remove old cruft out first ... */
    settings.beginWriteArray(QLatin1String("custom_cookies"));
    for (int i = 0; i < this->ui->CustomCookieRules_treeWidget->topLevelItemCount(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue(QLatin1String("flags"),(int)this->ui->CustomCookieRules_treeWidget->topLevelItem(i)->flags());
        settings.setValue(QLatin1String("action"),this->ui->CustomCookieRules_treeWidget->topLevelItem(i)->text(0));
        settings.setValue(QLatin1String("domain"),this->ui->CustomCookieRules_treeWidget->topLevelItem(i)->text(1));
        settings.setValue(QLatin1String("path"),this->ui->CustomCookieRules_treeWidget->topLevelItem(i)->text(2));
        settings.setValue(QLatin1String("name-value"),this->ui->CustomCookieRules_treeWidget->topLevelItem(i)->text(3));
    }
    settings.endArray();
}
