/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef FTPREPLY_H
#define FTPREPLY_H

#include <QFtp>
#include <QNetworkReply>
#include <QStringList>

class QFtp;

class FtpReply : public QNetworkReply
{
    Q_OBJECT
public:
    FtpReply(const QUrl &url);
    void            abort();
    qint64          bytesAvailable() const;
    bool            isSequential() const;
protected:
    qint64          readData(char *data, qint64 maxSize);
private slots:
    void            processCommand(int command, bool error);
    void            processListInfo(const QUrlInfo &urlInfo);
    void            processData();
private:
    void            setContent();
    void            setListContent();
    QFtp*           ftp;
    QList<QUrlInfo> items;
    QByteArray      content;
    qint64          offset;
    QStringList     units;
};

#endif // FTPREPLY_H
