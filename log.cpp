/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "log.h"
#include "ui_log.h"

#include <QMimeDatabase>
#include <QMimeType>
#include <QEventLoop>
#include <QSettings>
#include <QFileDialog>

Log::Log(QWidget *parent) :
    QWidget(parent),settingsFile(QLatin1String("config.ini")),
    ui(new Ui::Log)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);

    this->loadSettings();

    /* FIXME: Add error checking for logfile opening and also send notification
     * to user in case of error */
    if(this->ui->logToFile_checkBox->isChecked() && !this->ui->logFile_lineEdit->text().isEmpty() && \
            !this->ui->logFile_lineEdit->text().isNull()) {

        this->logFile.setFileName(this->ui->logFile_lineEdit->text());
        this->logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);
        this->out.setDevice(&this->logFile);
        this->out.setFieldAlignment(QTextStream::AlignLeft);
    }

    /* FIXME: We are using separate QNetworkAccessManager for logging.
     *
     * Ideally, we should use just one QNetworkAccessManager, the one provided by Browser class.
     *
     * The problem is, there is not just one instance of QNetworkAccessManager for
     * Browser class. There is an instance of QNetworkAccessManager for each open tab!

     * This was by design to make it easier to implement tracker blocker, cookie blocker etc....
     * (at the time it feeled like a right decision and it was super easy to do also)

     * So, you first have to rewrite the Browser class, make it use single, shared QNetworkAccessManager,
     * make all the tracker blocker, cookie blocker etc.. use that one, keep track of each of them
     * for each tab (so that you know what tap did block what tracker/blocker etc..) and
     * only THEN can you get rid of the QNetworkAccessManager below (yeah, it sucks)
     *
     * BTW: Download manager class also has this problem. That is, it also use separate instance
     * of QNetworkAccessManager while in reality it should use shared QNetworkAccessManager
     * provided by Browser class. This is also why, when using HTML Basic Authentication and trying
     * to download some file, it won't work because the authentication cookies are not shared
     * between QNetworkAccessManagers right now */

    this->manager = new QNetworkAccessManager(this);

    this->ui->Log_treeWidget->sortByColumn(0,Qt::AscendingOrder);
}

Log::~Log()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->logFile.close();
    this->saveSettings();
    delete ui;
}

void
Log::loadSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("log"));

    this->ui->enableLog_checkBox->setChecked(settings.value(QLatin1String("enabled")).toBool());
    this->ui->logToFile_checkBox->setChecked(settings.value(QLatin1String("logfile_enabled")).toBool());

    this->ui->enableLog_checkBox->toggled(settings.value(QLatin1String("enabled")).toBool());
    this->ui->logToFile_checkBox->toggled(settings.value(QLatin1String("logfile_enabled")).toBool());

    this->ui->logFile_lineEdit->setText(settings.value(QLatin1String("logfile")).toString());

    this->ui->dateTimeLog_checkBox->setChecked(settings.value(QLatin1String("log_datetime")).toBool());
    this->ui->actionLog_checkBox->setChecked(settings.value(QLatin1String("log_action")).toBool());
    this->ui->mimeTypeLog_checkBox->setChecked(settings.value(QLatin1String("log_mimetype")).toBool());

    settings.endGroup();

}

void
Log::saveSettings()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QSettings settings(this->settingsFile, QSettings::IniFormat);

    settings.beginGroup(QLatin1String("log"));

    settings.setValue(QLatin1String("enabled"),this->ui->enableLog_checkBox->isChecked());
    settings.setValue(QLatin1String("logfile_enabled"),this->ui->logToFile_checkBox->isChecked());
    settings.setValue(QLatin1String("logfile"),this->ui->logFile_lineEdit->text());

    settings.setValue(QLatin1String("log_datetime"),this->ui->dateTimeLog_checkBox->isChecked());
    settings.setValue(QLatin1String("log_action"),this->ui->actionLog_checkBox->isChecked());
    settings.setValue(QLatin1String("log_mimetype"),this->ui->mimeTypeLog_checkBox->isChecked());

    settings.endGroup();

}


void
Log::dataReady(const QStringList& data,const QColor& color)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QTreeWidgetItem*    item = new QTreeWidgetItem(this->ui->Log_treeWidget);

    item->setText(0,data.at(0));    // Date/Time
    item->setText(1,data.at(1));    // Action
    item->setText(2,data.at(2));    // MIME Type
    item->setText(3,data.at(3));    // URL

    register size_t columns = this->ui->Log_treeWidget->columnCount();
    for(register unsigned int i = 0;i < columns;++i) {
        item->setTextColor(i,color);
    }
    this->ui->Log_treeWidget->resizeColumnToContents(columns-1);

    this->ui->Log_treeWidget->scrollToBottom();

    /* Log to log file too... */
    if(this->ui->logToFile_checkBox->isChecked() && !this->ui->logFile_lineEdit->text().isEmpty() && \
            !this->ui->logFile_lineEdit->text().isNull()) {

        this->out.setFieldWidth(30);

        if(this->ui->dateTimeLog_checkBox->isChecked()) {
            out << data.at(0);
        }

        if(this->ui->actionLog_checkBox->isChecked()) {
            out << data.at(1);
        }

        if(this->ui->mimeTypeLog_checkBox->isChecked())     {
            out << data.at(2);
        }

        out << qSetFieldWidth(0) << data.at(3) << endl;
    }

}

void Log::on_clearLog_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->ui->Log_treeWidget->clear();
}

void
Log::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    this->ui->image_progressBar->setMaximum(bytesTotal);
    this->ui->image_progressBar->setValue(bytesReceived);
}

void
Log::finished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QNetworkReply*  reply = qobject_cast<QNetworkReply*>(sender());

    if(contentType.startsWith("image/"))
    {

        QTextCursor    cursor = this->ui->textEdit->textCursor();
        cursor.insertImage(QImage::fromData(reply->readAll()));

    }
    if(contentType.startsWith("application/javascript") || \
            contentType.startsWith("application/x-javascript") || \
            contentType.startsWith("text/") || \
            contentType.startsWith("application/json"))
    {
        this->ui->textEdit->setPlainText(reply->readAll());
    }
}

void Log::on_Log_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if(current) {

       this->ui->textEdit->clear();
       this->ui->image_progressBar->setValue(0);

       QEventLoop  loop;
       QNetworkReply*   check = this->manager->head(QNetworkRequest(QUrl(current->text(3))));
       connect(check,SIGNAL(finished()),&loop,SLOT(quit()));
       loop.exec();

       contentType = check->header(QNetworkRequest::ContentTypeHeader).toString();

       // "video/x-flv" is mimetype for flash video
       qDebug() << "MIMETYPE IS " << contentType;


    if(contentType.startsWith("application/javascript") || \
                   contentType.startsWith("application/x-javascript") || \
                   contentType.startsWith("text/") || \
                   contentType.startsWith("application/json") || \
                   contentType.startsWith("image/"))
       {
            QNetworkReply*  reply = this->manager->get(QNetworkRequest(QUrl(current->text(3))));
            connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));
            connect(reply,SIGNAL(finished()),this,SLOT(finished()));

        }
    }
}

void Log::on_browseLogFile_pushButton_clicked()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QString filterString = tr("Any (*.*)");
    QString filename = QFileDialog::getSaveFileName(this, tr("Save to Log File"),
                               QDir::currentPath() + "/log.txt",
                               filterString);

    if(!filename.isNull() && !filename.isEmpty()) {
        QFileInfo   fileInfo(filename);

        if(fileInfo.dir() == QDir::currentPath()) {
            this->ui->logFile_lineEdit->setText("./" + fileInfo.fileName());
        } else {
            this->ui->logFile_lineEdit->setText(filename);
        }

        this->logFile.close();
        this->logFile.setFileName(this->ui->logFile_lineEdit->text());

        /* FIXME: Add error checking for logfile opening and also send notification
         * to user in case of error */
        this->logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text);

        this->out.setDevice(&this->logFile);
        this->out.setFieldAlignment(QTextStream::AlignLeft);
    }

}
