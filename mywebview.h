/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef MYWEBVIEW_H
#define MYWEBVIEW_H

#include <QWebView>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QMimeData>

class MainWindow;

/* Note to self:
The downloadRequested(QNetworkRequest) signal is only emitted when you right click on the link, and then click on
the "Save link..." item of the context menu.

When the user clicks on a link to something that is supposed to be downloaded
(with the HTTP field Content-Disposition: attachment; filename=...),
QWebPage emits the unsupportedContent(QNetworkReply*) signal.
*/

class MyWebView : public QWebView
{
    Q_OBJECT
public:
    explicit        MyWebView(QWidget* parent = NULL);
    void            contextMenuEvent(QContextMenuEvent * event);
    QWebView*       createWindow(QWebPage::WebWindowType type);
    MainWindow*     getMainWindow();
    void            setMainWindow(MainWindow* m);
public slots:
    void            openLinkInNewTab();
    void            openImage();
private:
    QPoint          rel_pos;
    QMenu*          menu;
    QAction*        action_saveImage;
    QAction*        action_saveFile;
    QAction*        action_copyImage;
    QAction*        action_openLinkInNewWindow;
    QUrl            contextUrl;
    MainWindow*     mainWindow;
    bool            linkUrl;
    bool            imageUrl;
    bool            newTab;
    bool            newWindow;
};

#endif // MYWEBVIEW_H
