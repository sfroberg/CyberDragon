/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "dialog.h"
#include "ui_dialog.h"

#include <QtGlobal>
#include <QtWebKit>
#include <QSslSocket>

#include <zlib.h>
#include <GeoIP.h>



Dialog::Dialog(QWidget *parent) :
    QDialog(parent,Qt::CustomizeWindowHint|Qt::WindowTitleHint),
    ui(new Ui::Dialog)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    ui->setupUi(this);

    QString text = "<table align=center>";

    text += "<tr><th colspan=3>This is CyberDragon Browser - ";
    text += QLatin1Literal(CYBERDRAGON_VERSION) + "</th></tr>";
    text += "<tr><td align=center colspan=3>Copyright © 2013 - 2015 Stefan Fröberg</td></tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td align=center colspan=3>To make this better, please consider donating by visiting:</td></tr>";
    text += "<tr><td align=center colspan=3>http://www.binarytouch.com</td></tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td align=center colspan=3>Improvements and bug reports are wellcome to:</td></tr>";
    text += "<tr><td align=center colspan=3>http://www.binarytouch.com/contact.php</td><tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td align=center colspan=3>The CyberDragon Browser uses the following software components:</td></tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td colspan=2><b>For GUI & Web page rendering:</b></td></tr>";
    text += "<tr><td >Qt5</td><td>" + QLatin1Literal(qVersion()) + "</td></tr>";
    text += "<tr><td >WebKit</td><td>" + qWebKitVersion() + "</td></tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td colspan=2><b>For HTTPS & Proxy SSL support:</b></td></tr>";
    text += "<tr><td >OpenSSL</td><td>" + QSslSocket::sslLibraryVersionString() + "</td></tr>";

    text += "<tr><td></td><td></td></tr>";
    text += "<tr><td colspan=2><b>For Proxy geolocation checking:</b></td></tr>";
    text += "<tr><td>GeoIP</td><td>" + QLatin1Literal(GeoIP_lib_version()) + "</td></tr>";
    text += "<tr><td>Zlib</td><td>" + QLatin1Literal(ZLIB_VERSION) + "</td></tr>";
    text += "</table>";

    this->ui->label->setPixmap(QPixmap(":/icons/128x128/CyberDragonLogo.png"));
    this->ui->textEdit->setHtml(text);
}

Dialog::~Dialog()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";
    delete ui;
}
