/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef DOWNLOADS_H
#define DOWNLOADS_H

#include <QWidget>
#include <QTreeWidgetItem>
#include <QStyledItemDelegate>
#include <QNetworkReply>
#include <QUrl>
#include <QPointer>
#include <QFile>


class MainWindow;

namespace Ui {
class downloads;
}

class downloads : public QWidget
{
    Q_OBJECT
public:
    explicit downloads(MainWindow* m, QWidget *parent = 0);
    ~downloads();    
    MainWindow*     getMainWindow();
private slots:
    void            on_torrentView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
private:
    friend class    MainWindow;
    MainWindow*     mainWindow;
    Ui::downloads*  ui;
};

namespace Download {
    enum {
        FileName_Col,
        FileSize_Col,
        Progress_Col,
        StartStopButton_Col,
        Status_Col
    };
}

class TorrentViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    TorrentViewDelegate(QObject *mainWindow = NULL);
    bool            editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,const QModelIndex &index);
    void            paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
signals:
    void            buttonClicked(const QModelIndex &index);
private:
    QStyle::State   _state;
    int             _currentRow;
};


class TorrentClient : public QObject
 {
     Q_OBJECT
public:
    TorrentClient(const QUrl& url,const QString& fileName,QObject *parent = NULL);
    TorrentClient(QNetworkReply*& reply,const QString& fileName,QObject *parent = NULL);
    /* */
    qint64              progress() const;
    bool                isFinished()const;
    bool                isStopped()const;
public slots:
    void                        start();
    void                        start(QNetworkReply*& reply);
    void                        stop();
    void                        downloadInProgress(qint64 _bytesReceived, qint64 _totalBytes);
    void                        downloadFinished(QNetworkReply* reply);
    void                        downloadFinished();
signals:
    void                        progressUpdated(int);
    void                        progressFinished(QNetworkReply*);
    void                        progressFailed(QNetworkReply*);
    void                        stopDownload();
    void                        downloadedBytes(qint64, qint64);
private:
    QNetworkAccessManager       downloadManager;
    QUrl                        url;
    QString                     fileName;
    qint64                      bytesReceived;
    qint64                      totalBytes;
    bool                        stopped;
    bool                        finished;
    QPointer<QNetworkReply>     file_reply;
    QNetworkRequest             file_request;
    QFile                       file;
    QByteArray                  data;
};


#endif // DOWNLOADS_H
