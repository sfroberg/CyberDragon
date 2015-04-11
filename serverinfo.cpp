/* Original author: Copyright (c) 2013 - 2015 Stefan Fröberg */

/* This work has been released under GNU GPL version 2 or any later version.
 * Please see http://www.gnu.org/licenses/gpl-2.0.html or the provided gpl-2.0.txt file. */

#include "serverinfo.h"
#include "ui_serverinfo.h"

#include "ui_mainwindow.h"
#include "mainwindow.h"

#include <QSettings>

ServerInfo::ServerInfo(MainWindow* m, QWidget *parent) :
    QWidget(parent),mainWindow(m),gi(NULL),latitude(0.0),longitude(0.0),
    settingsFile(QLatin1String("config.ini")),
    ui(new Ui::ServerInfo)
{

    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    ui->setupUi(this);


/*****************************************/
/* GeoIP sanity checking */
/*****************************************/

/* FIXME: Proxy checker module (worker.cpp) already loads GeoIP.dat databse.
 *
 * AFAIK, GeoLiteCity.dat also provides country like GeoIP.dat.
 * So in theory you should only need GeoLiteCity.dat database and not both
 * (GeoIP.dat and GeoLiteCity.dat). Otherwise this is a waste of disk space and RAM.
 *
 * Just have to decide where to load & initialize GeoLiteCity.dat so that *both*
 * modules (this and worker.cpp)can share it ... */

    if(this->gi == NULL) {
        this->gi = GeoIP_open("./GeoLiteCity.dat",GEOIP_INDEX_CACHE | GEOIP_CHECK_CACHE);
        if(gi == NULL) {
            qDebug() << "Error opening database!!!";
        }

        /* make sure GeoIP deals with invalid query gracefully */
        if (GeoIP_country_code_by_addr(this->gi, NULL) != NULL) {
            qDebug() <<  "Invalid Query test failed, got non NULL, expected NULL";
        }

        if (GeoIP_country_code_by_name(this->gi, NULL) != NULL) {
            qDebug() << "Invalid Query test failed, got non NULL, expected NULL";
        }
    }
}

ServerInfo::~ServerInfo()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    delete ui;
}

void
ServerInfo::reverselookUp(const QHostInfo &host)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }
}

void
ServerInfo::lookUp(const QHostInfo &host)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    if (host.error() != QHostInfo::NoError) {
        qDebug() << "Lookup failed:" << host.errorString();
        return;
    }

    QString s;
    if(!host.addresses().isEmpty()) {

        s += QLatin1String("IP addresses: ");

        for(int i = 0;i < host.addresses().count();++i) {
            s += host.addresses().at(i).toString() + QLatin1String("\n");
        }

        GeoIPRecord* record = GeoIP_record_by_addr(gi,host.addresses().first().toString().toLatin1().constData());
        if(record) {

        const char* returnedCountry = record->country_name;

        s += QLatin1String("Country: ") + returnedCountry + QLatin1String("\n");
        s += QLatin1String("City: ") + QLatin1String(record->city)  + QLatin1String("\n");
        s += QLatin1String("Latitude: ") + QString::number(record->latitude)  + QLatin1String("\n");
        s += QLatin1String("Longitude: ") + QString::number(record->longitude)  + QLatin1String("\n");
        s += QLatin1String("Region: ") + QLatin1String(record->region) + QLatin1String("\n");
        s += QLatin1String("Postal code: ") + QLatin1String(record->postal_code) + QLatin1String("\n");

        }
        /* Save for later use */
        this->latitude = record->latitude;
        this->longitude = record->longitude;

        if(this->mainWindow->ui->googleMapsLookup_checkBox->isChecked()) {
            this->loadGoogleMaps();
        }

        this->ui->plainTextEdit->setPlainText(s);
    }
}

void ServerInfo::on_webView_loadFinished(bool arg1)
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->ui->progressBar->setValue(0);
}


void
ServerInfo::loadGoogleMaps()
{
    qDebug() << __PRETTY_FUNCTION__ << " called ...";

    this->url = "https://maps.googleapis.com/maps/api/staticmap?center=";
    this->url += QString::number(this->latitude) + QLatin1String(",") + QString::number(this->longitude) + "&zoom=" + QString::number(this->mainWindow->ui->zoom_spinBox->value()) + "&size=" + QString::number(this->mainWindow->ui->width_spinBox->value()) + "x" + QString::number(this->mainWindow->ui->height_spinBox->value()) + "&scale=" + QString::number(this->mainWindow->ui->scale_spinBox->value()) + "&maptype=" + this->mainWindow->ui->mapType_comboBox->currentText() + QLatin1String("&markers=color:red|label:S|") + QString::number(this->latitude) + QLatin1String(",") + QString::number(this->longitude);
    this->ui->webView->load(QUrl(url));
}
