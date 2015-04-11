/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */


#include "downloads.h"
#include "ui_downloads.h"

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileInfo>

static const unsigned long KILOBYTE = 1024;
static const unsigned long MEGABYTE = 1048576;
static const unsigned long GIGABYTE = 1073741824;
static const qint64 TERABYTE = 1099511627776;

QString bytes2String(qint64 bytes)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if(bytes < KILOBYTE){
        return QString::number(bytes) + " B";
    } else if (bytes < MEGABYTE) {
        return QString::number((static_cast<double>(bytes)/KILOBYTE),'f',2) + " KB";
        } else if (bytes < GIGABYTE) {
            return QString::number((static_cast<double>(bytes)/MEGABYTE),'f',2) + " MB";
            } else if (bytes < TERABYTE) {
            return QString::number((static_cast<double>(bytes)/GIGABYTE),'f',2) + " GB";
            } else {
                return QString::number((static_cast<double>(bytes)/TERABYTE),'f',2) + " TB";
            }
}

downloads::downloads(MainWindow* m, QWidget *parent) :
    QWidget(parent),mainWindow(m),
    ui(new Ui::downloads)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    ui->setupUi(this);
}

downloads::~downloads()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    delete ui;
}

MainWindow*
downloads::getMainWindow()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return  this->mainWindow;
}

void downloads::on_torrentView_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if(current) {
        QFileInfo fileInfo(current->toolTip(0));
        QMimeDatabase   mimeDatabase;
        QMimeType   mimeType = mimeDatabase.mimeTypeForFile(fileInfo);

        QFile   file(current->toolTip(0));
        QString text = "Name:\t" + fileInfo.fileName() + \
                       "\nPath:\t" + fileInfo.path() + \
                        "\nSize:\t" + bytes2String(fileInfo.size()) + \
                        "\nMime Type:\t" + mimeType.name() + \
                        "\nSuffixes:\t" + mimeType.suffixes().join(", ") + \
                        "\nDescription:\t" + mimeType.comment();

        if(file.open(QIODevice::ReadOnly)) {
            QByteArray  data = file.readAll();
            text += "\nSHA1:\t" + QString(QCryptographicHash::hash(data,QCryptographicHash::Sha1).toHex());
        }
        this->getMainWindow()->ui->fileInformation_plainTextEdit->setPlainText(text);
    }
}



TorrentViewDelegate::TorrentViewDelegate(QObject *mainWindow) :
    QStyledItemDelegate(mainWindow)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    _state =  QStyle::State_Enabled;
    _currentRow = -1;
}

bool TorrentViewDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option,const QModelIndex &index)
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
                if(index.column() == Download::StartStopButton_Col)
                    emit buttonClicked(index);
            }
            break;
            default:
                return QStyledItemDelegate::editorEvent(event,model,option,index);
        }
    }
    return QStyledItemDelegate::editorEvent(event,model,option,index);
}

void TorrentViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    if (!index.isValid())
        return;

    switch(index.column()) {
        case    Download::Progress_Col:
        {
            // Set up a QStyleOptionProgressBar to precisely mimic the
            // environment of a progress bar.
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.state = QStyle::State_Enabled;
            progressBarOption.direction = QApplication::layoutDirection();
            progressBarOption.rect = option.rect;
            progressBarOption.fontMetrics = QApplication::fontMetrics();
            progressBarOption.minimum = 0;
            progressBarOption.maximum = 100;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.textVisible = true;

            // Set the progress and text values of the style option.
            int progress = qobject_cast<MainWindow *>(parent())->clientForRow(index.row())->progress();
            progressBarOption.progress = progress < 0 ? 0 : progress;
            progressBarOption.text = QString().sprintf("%d%%", progressBarOption.progress);

            // Draw the progress bar onto the view.
            QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
        }
        break;
        case Download::StartStopButton_Col:
        {

         QStyleOptionButton button;
         button.features |= QStyleOptionButton::Flat;

         button.rect = option.rect;
         button.rect.setHeight(16);
         button.rect.setWidth(16+10);
         button.iconSize = QSize(16,16);

         bool   stopped = qobject_cast<MainWindow *>(parent())->clientForRow(index.row())->isStopped();
         bool   finished = qobject_cast<MainWindow *>(parent())->clientForRow(index.row())->isFinished();
         button.state = QStyle::State_Enabled;
        if( (stopped && !finished) || (stopped && finished) ) {
             button.icon = QIcon(QLatin1String(":/icons/16x16/media-playback-start-3.png"));
         } else {
             button.icon = QIcon(QLatin1String(":/icons/16x16/media-playback-stop-3.png"));
         }
         QApplication::style()->drawControl(QStyle::CE_PushButton, &button, painter);
        }
        break;
        default:
            QStyledItemDelegate::paint(painter, option, index);
    }
}

/*********************************************************/

TorrentClient::TorrentClient(const QUrl& url,const QString& fileName,QObject *parent):
    QObject(parent),url(url),fileName(fileName),bytesReceived(0),totalBytes(0),stopped(true),finished(false),file_reply(NULL)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->file_request.setUrl(this->url);
    start();
}

TorrentClient::TorrentClient(QNetworkReply*& reply,const QString& fileName,QObject *parent):
    QObject(parent),fileName(fileName),bytesReceived(0),totalBytes(0),stopped(true),finished(false),file_reply(reply)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->url = reply->url();
    this->file_request = this->file_reply.data()->request();
    this->file_request.setUrl(this->url);

    start();
}

/* Give value between 0 - 100 */
qint64 TorrentClient::progress() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return (totalBytes > 0 ? ((static_cast<double>(bytesReceived) / static_cast<double>(totalBytes)) * 100) : 0);
}

bool    TorrentClient::isFinished() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return this->finished;
}

bool    TorrentClient::isStopped() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return this->stopped;
}

void    TorrentClient::start()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    stopped = false;
    finished = false;
    this->bytesReceived = 0;
    this->totalBytes = 0;

    qDebug() << "URL " << url.toString() << " FILENAME " << fileName;

    if(!this->file_reply) {
        QNetworkReply*  reply = downloadManager.get(this->file_request);

        connect(&downloadManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(downloadFinished(QNetworkReply*)));
        connect(this,SIGNAL(stopDownload()),reply,SLOT(abort()));
        connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadInProgress(qint64,qint64)));
        connect(reply,SIGNAL(destroyed()),reply,SLOT(deleteLater()));

    } else {

        connect(this->file_reply,SIGNAL(finished()),this,SLOT(downloadFinished()));
        connect(this,SIGNAL(stopDownload()),this->file_reply,SLOT(abort()));
        connect(this->file_reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadInProgress(qint64,qint64)));
        connect(this->file_reply,SIGNAL(destroyed()),this->file_reply,SLOT(deleteLater()));
    }
}

void    TorrentClient::start(QNetworkReply*& reply)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    stopped = false;
    finished = false;
    this->bytesReceived = 0;
    this->totalBytes = 0;

    qDebug() << "URL " << url.toString() << " FILENAME " << fileName;

    connect(reply,SIGNAL(finished()),this,SLOT(downloadFinished()));
    connect(this,SIGNAL(stopDownload()),reply,SLOT(abort()));
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadInProgress(qint64,qint64)));
    connect(reply,SIGNAL(destroyed()),reply,SLOT(deleteLater()));
}

void    TorrentClient::stop()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "THIS IS TORCLIENT: STOPPING DOWNLOAD";

    stopped = true;
    emit stopDownload();
}

void    TorrentClient::downloadInProgress(qint64 _bytesReceived, qint64 _totalBytes)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << "BYTES1 " << _bytesReceived << "/" << _totalBytes;
    qDebug() << "BYTES2 " << ((static_cast<double>(_bytesReceived) / static_cast<double>(_totalBytes)) * 100) << "%";

    this->bytesReceived = _bytesReceived;
    this->totalBytes = _totalBytes;

    emit downloadedBytes(_bytesReceived,_totalBytes);
    emit progressUpdated((_totalBytes > 0 ? (( static_cast<double>(_bytesReceived) / static_cast<double>(_totalBytes)) * 100) : 0));
}

void    TorrentClient::downloadFinished(QNetworkReply* reply)
{
    qDebug() << __FUNCTION__ << " FINISHED DOWNLOADING";

    switch(reply->error()) {
        case    QNetworkReply::NoError:
        {
            qDebug() << "Everything went OK";
            stopped = true;
            finished = true;
            qDebug() << "==================================================================";
            qDebug() << "BYTES RECEIVED " << this->bytesReceived;
            qDebug() << "TOTAL BYTES " <<this->totalBytes;

            this->file.setFileName(this->fileName);
            if(this->file.open(QIODevice::WriteOnly)) {
                QByteArray data = reply->readAll();
                this->file.write(data);
                qDebug() << "WRITED " << data.size() << "BYTES";
            }
            this->file.close();
            this->downloadManager.disconnect();  /* To make sure that file is written to disk */

            emit progressFinished(reply);
        }
        break;
        case    QNetworkReply::OperationCanceledError:
        {
            qDebug() << "SOMETHING HAPPENED " << reply->errorString();
            this->bytesReceived = 0;
            this->totalBytes = 0;

            emit progressFailed(reply);
        }
        break;
        default:
            stopped = true;
            finished = false;
            this->bytesReceived = 0;
            this->totalBytes = 0;

            qDebug() << "SOMETHING HAPPENED " << reply->errorString();

            emit progressFailed(reply);
    }
}

/* ============================= */
void    TorrentClient::downloadFinished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    switch(reply->error()) {
        case    QNetworkReply::NoError:
        {

            qDebug() << "Everything went OK";

            stopped = true;
            finished = true;
            qDebug() << "==================================================================";
            qDebug() << "BYTES RECEIVED " << this->bytesReceived;
            qDebug() << "TOTAL BYTES " << this->totalBytes;

            this->file.setFileName(this->fileName);
            if(this->file.open(QIODevice::WriteOnly)) {
                QByteArray data = reply->readAll();
                this->file.write(data);

                qDebug() << "WRITED " << data.size() << "BYTES";

            }
            this->file.close();
            reply->close();

            emit progressFinished(reply);
        }
        break;
        case    QNetworkReply::OperationCanceledError:
        {

            qDebug() << "SOMETHING HAPPENED " << reply->errorString();

            this->bytesReceived = 0;
            this->totalBytes = 0;

            emit progressFailed(reply);
        }
        break;
        default:
            stopped = true;
            finished = false;
            this->bytesReceived = 0;
            this->totalBytes = 0;

            qDebug() << "SOMETHING HAPPENED " << reply->errorString();

            emit progressFailed(reply);
    }
}

