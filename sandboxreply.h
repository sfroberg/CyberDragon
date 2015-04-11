/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef SANDBOXREPLY_H
#define SANDBOXREPLY_H

#include <QNetworkReply>
#include <QStringList>

class SandboxReply : public QNetworkReply {
    Q_OBJECT
public:
    SandboxReply(QObject* parent, QNetworkReply* reply);
    ~SandboxReply();
    void        abort();
    void        close();
    bool        isSequential() const;
    void        setReadBufferSize(qint64 size);
    qint64      bytesAvailable() const;
    qint64      bytesToWrite() const;
    qint64      readData(char* data, qint64 maxlen);
public slots:
    void        ignoreSslErrors();
    void        applyMetaData();
    void        errorInternal(QNetworkReply::NetworkError _error);
    void        readInternal();
    void        slot_Finished();
    void        doStuff();
    void        setContent( const QByteArray &content );
private:
    QNetworkReply*  m_reply;
    QByteArray      m_data;
    QByteArray      m_buffer;
    //QByteArray      m_tmp;
    QString m_tmp;
};

#endif // SANDBOXREPLY_H
