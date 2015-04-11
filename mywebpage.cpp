/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "mywebpage.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "general.h"
#include "ui_general.h"

#include <QMessageBox>
#include <QDesktopServices>


MyWebPage::MyWebPage(MainWindow* m, QObject * parent):
        QWebPage(parent),mainWindow(m)
{
    this->setForwardUnsupportedContent(true);
}

QString
MyWebPage::userAgentForUrl(const QUrl & url) const
{
    Q_ASSERT(this->mainWindow != NULL);
    Q_ASSERT(this->mainWindow->generalTab != NULL);

    if(!this->mainWindow->generalTab->ui->UserAgent_lineEdit->text().isEmpty()) {
        return  this->mainWindow->generalTab->ui->UserAgent_lineEdit->text();
    }

    /* Otherwise return default value */
    return  QWebPage::userAgentForUrl(url);
}

bool
MyWebPage::acceptNavigationRequest(QWebFrame * frame, const QNetworkRequest & request, NavigationType type)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << request.url();

    /* Opens default bittorrent app for magnet URI scheme.

     * CyberDragon really needs prober MIMETYPE handling for unrecognized content!
     * FIXME: Add MIMETYPE tab where user can define default apps that is used to open content.

     * Also, add GUI dialog where user can decide what default app to use when
     * unrecognized content is encountered and it is missing from previously mentioned
     * MimeTypes tab */
    if(request.url().scheme() == "magnet") {
        QDesktopServices::openUrl(request.url());
        return false;
    }

    if (type == NavigationTypeLinkClicked) {
        switch (this->linkDelegationPolicy()) {
            case DelegateAllLinks:
            {
                /* If page tries to open new tab (frame == NULL) then bypass handling of
                 * linkClicked() signal */
                if(frame == NULL) {
                    qDebug() << type;
                    return true;
                }
                emit linkClicked(request.url());
                return  false;
            }
            default:
                return  QWebPage::acceptNavigationRequest(frame,request,type);
        }
    }
    return true;
}


