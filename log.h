/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#ifndef LOG_H
#define LOG_H

#include <QNetworkAccessManager>
#include <QTreeWidgetItem>
#include <QNetworkReply>
#include <QFile>

namespace Ui {
class Log;
}

class Log : public QWidget
{
    Q_OBJECT
public:
    explicit Log(QWidget *parent = NULL);
    ~Log();
    void                    loadSettings();
    void                    saveSettings();
public slots:
    void                    dataReady(const QStringList& data,const QColor& color);
private slots:
    void                    on_clearLog_pushButton_clicked();
    void                    downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void                    finished();
    void                    on_Log_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void                    on_browseLogFile_pushButton_clicked();
private:
    friend class            MyNetworkAccessManager;
    friend class            Browser;
    QNetworkAccessManager*  manager;
    QString                 settingsFile;
    QString                 contentType;
    QFile                   logFile;
    QTextStream             out;
    Ui::Log*                ui;
};

#endif // LOG_H
