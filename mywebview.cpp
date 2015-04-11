/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "mywebview.h"

#include <QMessageBox>
#include <QMenu>
#include <QWebHitTestResult>
#include <QContextMenuEvent>

#include "browser.h"
#include "ui_browser.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

MyWebView::MyWebView(QWidget* parent):
    QWebView(parent),mainWindow(NULL),linkUrl(false), imageUrl(false), newTab(false),newWindow(false)
{
}

/* FIXME: Add menu entry for opening new CyberDragon window. Also see the next function */
void
MyWebView::contextMenuEvent(QContextMenuEvent * event)
{
    qDebug() << "=================================================";
    qDebug() << __FUNCTION__ << " called ...";

    this->rel_pos = event->pos();
    QWebHitTestResult r = page()->mainFrame()->hitTestContent(event->pos());

    QMenu menu(this);

    if (!r.linkUrl().isEmpty()) {
        menu.addAction(pageAction(QWebPage::OpenLink));
        menu.addAction(tr("Open Link in New Tab"), this, SLOT(openLinkInNewTab()));
        menu.addAction(pageAction(QWebPage::DownloadLinkToDisk));

        menu.addAction(pageAction(QWebPage::CopyLinkToClipboard));
        menu.addSeparator();
    }

    if(!r.imageUrl().isEmpty()) {
        menu.addAction(tr("Open Image in New Tab"),this,SLOT(openImage()));
        menu.addAction(pageAction(QWebPage::DownloadImageToDisk));
        menu.addAction(pageAction(QWebPage::CopyImageToClipboard));
        menu.addAction(pageAction(QWebPage::CopyImageUrlToClipboard));
    }

    if(!r.linkUrl().isEmpty() || !r.imageUrl().isEmpty()) {
        menu.exec(mapToGlobal(rel_pos));
        return;
    }
    QWebView::contextMenuEvent(event);
}

QWebView*
MyWebView::createWindow(QWebPage::WebWindowType type)
{
    qDebug() << "=================================================";
    qDebug() << __FUNCTION__ << " called ...";

    switch(type) {
        case    QWebPage::WebBrowserWindow:
        {
            QWebHitTestResult  hitTest = this->page()->mainFrame()->hitTestContent(rel_pos);
            QUrl    url;

            if(this->linkUrl && !hitTest.linkUrl().isEmpty()) {
                    url = hitTest.linkUrl();
                    this->linkUrl = false;
            }

            if(this->imageUrl && !hitTest.imageUrl().isEmpty()) {
                    url = hitTest.imageUrl();
                    this->imageUrl = false;
            }

            if(newWindow) {
                /* FIXME: Create new CyberDragon window instance here when
                 * user has selected "Open New Window" from context menu
                 * and return it */
                } else {
                    qDebug() << "url.toString(): " << url.toString();
                    Browser*    browser = this->getMainWindow()->addTab(url.toString());
                    qDebug() << "browser->getWebView()->url().toString(): " << browser->getWebView()->url().toString();
                    return browser->getWebView();
                }
        }
        break;
    }
    return  QWebView::createWindow(type);
}

MainWindow*
MyWebView::getMainWindow()
{
    return  this->mainWindow;
}

void
MyWebView::setMainWindow(MainWindow* m)
{
    this->mainWindow = m;
}

void
MyWebView::openLinkInNewTab()
{
    qDebug() << "=================================================";
    qDebug() << __FUNCTION__ << " called ...";

    this->linkUrl = true;
    this->newWindow = false;
    this->createWindow(QWebPage::WebBrowserWindow);
}

void
MyWebView::openImage()
{
    qDebug() << "=================================================";
    qDebug() << __FUNCTION__ << " called ...";

    this->imageUrl = true;
    this->newWindow = false;
    this->createWindow(QWebPage::WebBrowserWindow);
}
