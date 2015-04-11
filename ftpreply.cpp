/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include <QTextDocument>
#include <QtNetwork>
#include "ftpreply.h"
#include <QDebug>

/* The stuff in this module was taken pretty much verbatim from old Qt 4.x example for FTP .... */

FtpReply::FtpReply(const QUrl &url)
    : QNetworkReply()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ftp = new QFtp(this);
    connect(ftp, SIGNAL(listInfo(QUrlInfo)), this, SLOT(processListInfo(QUrlInfo)));
    connect(ftp, SIGNAL(readyRead()), this, SLOT(processData()));
    connect(ftp, SIGNAL(commandFinished(int, bool)), this, SLOT(processCommand(int, bool)));

    offset = 0;
    units = QStringList() << "bytes" << "K" << "M" << "G" << "Ti" << "Pi"
                          << "Ei" << "Zi" << "Yi";
    setUrl(url);
    ftp->connectToHost(url.host());
}

void FtpReply::processCommand(int, bool err)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if (err) {
        setError(ContentNotFoundError, "Unknown command");
        emit error(ContentNotFoundError);
        return;
    }

    switch (ftp->currentCommand()) {
    case QFtp::ConnectToHost:
        qDebug() << "Connecting to FTP ...";
        ftp->login();
        break;
    case QFtp::Login:
        qDebug() << "Login to FTP ...";
        ftp->list(url().path());
        break;

    case QFtp::List:
        qDebug() << "List FTP ...";
        if (items.size() == 1 && !items.at(0).isDir()) {
            ftp->get(url().path());
        } else {
            setListContent();
            qDebug() << this->content;
        }
        break;

    case QFtp::Get:
        setContent();
    default:
        ;
    }
}

void FtpReply::processListInfo(const QUrlInfo &urlInfo)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    items.append(urlInfo);
}

void FtpReply::processData()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    content += ftp->readAll();
}

void FtpReply::setContent()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    open(ReadOnly | Unbuffered);
    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(content.size()));
    emit readyRead();
    emit finished();
    ftp->close();
}

void FtpReply::setListContent()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    QUrl u = url();
    if (!u.path().endsWith(QLatin1String("/")))
        u.setPath(u.path() + "/");

    QString base_url = url().toString();
    QString base_path = u.path();

    open(ReadOnly | Unbuffered);

    QString content(
        "<html><head><title>" + base_url.toHtmlEscaped() + "</title><style type=\"text/css\">"
        "  th { background-color: #aaaaaa; color: black }\n"
        "  table { border: solid 1px #aaaaaa }\n"
        "  tr.odd { background-color: #dddddd; color: black\n }\n"
        "  tr.even { background-color: white; color: black\n }\n"
        "  </style></head><body><h1>Listing for " + base_path + "</h1><table align=\"center\" cellspacing=\"0\" width=\"90%\"><tr><th>Name</th><th>Size</th></tr>");


    QUrl parent = u.resolved(QUrl(QLatin1String("..")));

    if (parent.isParentOf(u))
        content += QString("<tr><td><strong><a href=\"" + parent.toString() + "\">Parent directory</a></strong></td><td></td></tr>");

    int i = 0;
    qDebug() << "FTP ITEMS COUNT " << items.count();
    foreach (const QUrlInfo &item, items) {
        qDebug() << item.name();
        QUrl child = u.resolved(QUrl(item.name()));

        if (i == 0)
            content += QString("<tr class=\"odd\">");
        else
            content += QString("<tr class=\"even\">");

        content += QString("<td><a href=\"" + child.toString() + "\">" + item.name().toHtmlEscaped() + "</a></td>");

        qint64 size = item.size();
        int unit = 0;
        while (size) {
            qint64 new_size = size/1024;
            if (new_size && unit < units.size()) {
                size = new_size;
                unit += 1;
            } else
                break;
        }

        if (item.isFile())
            content += QString("<td>" + QString::number(size) + " " + units[unit] + "</td></tr>");
        else
            content += QString("<td></td></tr>");

        i = 1 - i;
    }

    content += QString("</table></body></html>");
    this->content = content.toUtf8();

    setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/HTML; charset=UTF-8"));
    setHeader(QNetworkRequest::ContentLengthHeader, QVariant(this->content.size()));

    emit readyRead();
    emit finished();
    ftp->close();
}

// QIODevice methods

void FtpReply::abort()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
}

qint64 FtpReply::bytesAvailable() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return content.size() - offset;
}

bool FtpReply::isSequential() const
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    return true;
}

qint64 FtpReply::readData(char *data, qint64 maxSize)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    qDebug() << data;

    if (offset < content.size()) {
        qint64 number = qMin(maxSize, content.size() - offset);
        memcpy(data, content.constData() + offset, number);
        offset += number;
        return number;
    } else {
        return -1;
    }
}
