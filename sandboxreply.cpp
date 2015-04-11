/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "sandboxreply.h"

#include <QRegularExpression>
#include <QTimer>
#include <QWebFrame>
#include <QWebPage>
#include <QWebElement>
#include <QWebElementCollection>
#include <QTextStream>

/* Sandboxing of Iframes using custom QNetworkReply. Very experimental. Very messy. Very ugly.
 * Could not find docs how to do this properly, so this is a result of pieces from all over the Internet.

 * All the blocking stuff magic happens in slot_Finished() function but
 * maybe the right place would be readInternal() function ? ....
 * Experiment! */

SandboxReply::SandboxReply(QObject* parent, QNetworkReply* reply)
: QNetworkReply(parent)
, m_reply(reply)
{
   // m_reply->setReadBufferSize(0);  // Unlimited buffer!!! Is this going to be a problem???

    setOperation(m_reply->operation());
    setRequest(m_reply->request());
    setUrl(m_reply->url());

    qDebug() << "m_tmp starting size  " <<  m_tmp.size();

    connect(m_reply,SIGNAL(finished()),this,SLOT(slot_Finished()));

    connect(m_reply, SIGNAL(metaDataChanged()), SLOT(applyMetaData()));
    connect(m_reply, SIGNAL(readyRead()), SLOT(readInternal()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(errorInternal(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(finished()), SIGNAL(finished()));
    connect(m_reply, SIGNAL(uploadProgress(qint64,qint64)), SIGNAL(uploadProgress(qint64,qint64)));
    connect(m_reply, SIGNAL(downloadProgress(qint64,qint64)), SIGNAL(downloadProgress(qint64,qint64)));


    //connect(m_reply,SIGNAL(finished()),this,SLOT(doStuff()));

    //setOpenMode(ReadOnly);
    setOpenMode(ReadWrite);
}


SandboxReply::~SandboxReply()
{
    delete m_reply;
}

void
SandboxReply::abort()
{
    m_reply->abort();
}

void
SandboxReply::close()
{
    m_reply->close();
}

bool
SandboxReply::isSequential() const
{
    return m_reply->isSequential();
}

void
SandboxReply::setReadBufferSize(qint64 size)
{
    QNetworkReply::setReadBufferSize(size);
    m_reply->setReadBufferSize(size);
}

qint64
SandboxReply::bytesAvailable() const
{
    return m_buffer.size() + QIODevice::bytesAvailable();
}

qint64
SandboxReply::bytesToWrite() const
{
    return -1;
}

qint64
SandboxReply::readData(char* data, qint64 maxlen)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    qint64 size = qMin(maxlen, qint64(m_buffer.size()));
    memcpy(data, m_buffer.constData(), size);
    m_buffer.remove(0, size);

    return size;
}

void
SandboxReply::applyMetaData()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QList<QByteArray> headers = m_reply->rawHeaderList();
    foreach(QByteArray header, headers)
    setRawHeader(header, m_reply->rawHeader(header));
    setHeader(QNetworkRequest::ContentTypeHeader, m_reply->header(QNetworkRequest::ContentTypeHeader));
    setHeader(QNetworkRequest::ContentLengthHeader, m_reply->header(QNetworkRequest::ContentLengthHeader));

    setHeader(QNetworkRequest::LocationHeader, m_reply->header(QNetworkRequest::LocationHeader));
    setHeader(QNetworkRequest::LastModifiedHeader, m_reply->header(QNetworkRequest::LastModifiedHeader));
    setHeader(QNetworkRequest::SetCookieHeader, m_reply->header(QNetworkRequest::SetCookieHeader));
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute));
    setAttribute(QNetworkRequest::HttpReasonPhraseAttribute, m_reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute));
    setAttribute(QNetworkRequest::RedirectionTargetAttribute, m_reply->attribute(QNetworkRequest::RedirectionTargetAttribute));
    setAttribute(QNetworkRequest::ConnectionEncryptedAttribute, m_reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute));
    setAttribute(QNetworkRequest::CacheLoadControlAttribute, m_reply->attribute(QNetworkRequest::CacheLoadControlAttribute));
    setAttribute(QNetworkRequest::CacheSaveControlAttribute, m_reply->attribute(QNetworkRequest::CacheSaveControlAttribute));
    setAttribute(QNetworkRequest::SourceIsFromCacheAttribute, m_reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute));
    setAttribute(QNetworkRequest::DoNotBufferUploadDataAttribute, m_reply->attribute(QNetworkRequest::DoNotBufferUploadDataAttribute));
    emit metaDataChanged();
}

void
SandboxReply::ignoreSslErrors()
{
    m_reply->ignoreSslErrors();
}

void
SandboxReply::errorInternal(QNetworkReply::NetworkError _error)
{
    setError(_error, errorString());
    emit error(_error);
}

void
SandboxReply::readInternal()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    QVariant contentMimeType = m_reply->header(QNetworkRequest::ContentTypeHeader);

    qDebug() << m_reply->rawHeaderPairs();
    qDebug() << this->bytesAvailable() << "/" << this->size();
    qDebug() << m_reply->bytesAvailable() << "/" << m_reply->size();

    QByteArray data = m_reply->readAll();

    /*if(m_reply->rawHeader("Transfer-Encoding") == "chunked") {
        qDebug() << "CHUNKED ENCODING DETECTED";
        data.replace('\0',"");
    }*/

    m_data = data;
    m_tmp += m_data;
    qDebug() << "==================================";
    qDebug() << "data is ";
    qDebug() << "==================================";
    qDebug() << data;
    qDebug() << "==================================";
    qDebug() << "m_tmp size " << m_tmp.size();
    qDebug() << "==================================";

 //   QTextStream(stdout) << m_tmp << endl;

    if(m_tmp.isEmpty())
        qDebug() << "m_tmp is EMPTY!";
    if(m_tmp.isNull())
        qDebug() << "m_tmp is NULL!";

    qDebug() << "==================================";

    emit readyRead();
}


void
SandboxReply::doStuff()
{
    qDebug() << "===============================================================";
    qDebug() << m_data;
    m_buffer += m_data;
    qDebug() << "===============================================================";
    //m_data.clear();
}

void
SandboxReply::setContent( const QByteArray &content )
{
    m_buffer = content;
    //m_data += content;
}


void
SandboxReply::slot_Finished()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    qDebug() << m_buffer; //
    qDebug() << "================================";
    qDebug() << m_data; //
    qDebug() << this->bytesAvailable() << "/" << this->size();
    qDebug() << m_reply->bytesAvailable() << "/" << m_reply->size();
    qDebug() << "==========================================================";
    qDebug() << "m_tmp buffer size is  " << m_tmp.size();
    qDebug() << "==========================================================";

    if(m_tmp.isEmpty())
        qDebug() << "m_tmp is EMPTY!";
    if(m_tmp.isNull())
        qDebug() << "m_tmp is NULL!";

    qDebug() << m_tmp;

    qDebug() << "==========================================================";

    QVariant contentMimeType =this->header(QNetworkRequest::ContentTypeHeader);

/*    if(contentMimeType.isValid() && contentMimeType.toString().startsWith(QLatin1String("text/html"))) {
        m_buffer = m_tmp;
    }*/
    QWebPage    page;

    if(contentMimeType.isValid() && contentMimeType.toString().startsWith(QLatin1String("text/html"))) {
       //page.mainFrame()->setHtml(m_tmp);

        qDebug() << "=====================================";
        qDebug() << m_tmp;
        qDebug() << "=====================================";

        page.mainFrame()->setContent(m_tmp.toUtf8(),contentMimeType.toString());
        QWebElement doc = page.mainFrame()->documentElement();
        QWebElement iframe = doc.findFirst("iframe");
        iframe.setAttribute("sandbox","");

        qDebug() << "===================================";
        qDebug() << page.mainFrame()->toHtml().toLatin1();
        qDebug() << "===================================";
        //m_buffer = page.mainFrame()->toHtml().toLatin1();

        qDebug() << "=====================================";
        //m_tmp = page.mainFrame()->toHtml().toUtf8();
        m_tmp = page.mainFrame()->toHtml();
        qDebug() << m_tmp;

        m_buffer = m_tmp.toUtf8();
//        m_buffer = m_tmp.toUtf8();
//        m_tmp.replace(QRegularExpression(QLatin1String("(<iframe)")),QLatin1String("\\1 sandbox"));
        //m_tmp.remove(QRegularExpression(QLatin1String("<div id=\"ylabanneri\".*</div>")));
        //m_tmp.remove(QRegularExpression(QLatin1String("<div id=\"ylabanneri\".*</div>")));

       //m_buffer = m_tmp.toLatin1();
  //      m_buffer = m_tmp.toUtf8();
    }


    //qDebug() << m_tmp;
    /*for(int i = 0;i < m_tmp.size();++i) {
        qDebug() << m_tmp.at(i);
    }*/

    qDebug() << "===================================";
    /*foreach(QString x, m_tmp)
        QTextStream(stdout) << x << endl;*/

    /*for(int i = 0;i < m_tmp.size();++i) {
        QTextStream(stdout) << m_tmp.at(i) << endl;
    }*/
    qDebug() << "===================================";
    m_buffer = m_tmp.toUtf8();
}

