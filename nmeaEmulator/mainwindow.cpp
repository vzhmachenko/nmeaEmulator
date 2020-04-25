#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
                      : QWidget(parent) {
  initElements();
  connect(&ping, &QTimer::timeout,
          this, &MainWindow::slotTimerPing);
  ping.setInterval(1000);
  QList<QSerialPortInfo> list;
  list = QSerialPortInfo::availablePorts();



  /*
  horizontalLay = new QHBoxLayout;
  /*coordPainter.setPen(QPen(Qt::black, 12, Qt::DashDotLine, Qt::RoundCap));
  coordPainter.drawLine(QLineF(10.0, 80.0, 90.0, 20.0) );
  coordPainter.drawPixmap(0, 0, 400, 400, coordPixmap);

  coordLabel.setPixmap(coordPixmap);
  coordLabel.setFixedSize(400,400);
  */

//  horizontalLay->addWidget(&coordLabel);
//  formRightSight();
//  this->setLayout(horizontalLay);
}

MainWindow::~MainWindow() {

}
void :: MainWindow::slotTimerPing(){
  double sin_easting = sin(qDegreesToRadians(angle));
  double cos_northing = cos(qDegreesToRadians(angle));
  easting   = easting  + sin(qDegreesToRadians(angle)) * distanse;
  northing  = northing + cos(qDegreesToRadians(angle)) * distanse;

  slotConvert(false);
  QString nmeaSentence = QString("$GPGGA,123519,%1,%2,%3,%4,1,08,0.9,545.4,M,46.9,*47\n")
                          .arg(nmeaLat) .arg(north) .arg(nmeaLon) .arg(east);
  qint64 bytes = m_serial->write(QByteArray().append(nmeaSentence));
  qDebug() << "Tanslated " << bytes << " bytes.";
  if(bytes > 0){
    qDebug() << nmeaSentence;
  }
  /*
  static int letter = 48;
  QByteArray sendData;
  for(int k = 0; k < 2; ++k){
    sendData.append('$');
    for(int i = 0; i<60/5; ++i){
      sendData.append( QByteArray(5, letter++));
    }
    sendData.append('\n');
  }


  if(letter > 90)
    letter = 48;
  qint64 bytes = m_serial->write(sendData);
  qDebug() << "Tanslated " << bytes << " bytes.";
  if(bytes > 0){
    qDebug() << sendData;
  }
  */
}

void
MainWindow ::  connectToComPort(){
  m_serial = new QSerialPort;
  QString portName = QString("/dev/ttyUSB%1").arg(comPortNumber.value());
  m_serial->setPortName(portName);
  m_serial->setBaudRate(QSerialPort::Baud9600);
  m_serial->setDataBits(QSerialPort::Data8);
  m_serial->setParity(QSerialPort::EvenParity);
  m_serial->setStopBits(QSerialPort::OneStop);
  m_serial->setFlowControl(QSerialPort::NoFlowControl);

  if(m_serial->open(QIODevice::ReadWrite)){
    ping.start();
    connected = true;
  }
  else{
    qDebug() << "OpenError";
  }
}

void
MainWindow :: slotConvert(bool clicked){
  if(!connected){
    connectToComPort();
  }

  _zone->setText(QString::number(zone));
  utmLat->setText(QString::number(easting));
  utmLon->setText(QString::number(northing));

  if(!convertUTMtoLL(zone, easting, northing) ){
    wgsLatitude->setText(latitude);
    wgsLongtitude->setText(longitude);

    setlocale (LC_ALL,"C");

    double lat = latitude.toFloat();
    double lon = longitude.toFloat();
    double l1, l2, l3;


    l1 = int(lat);                             // Берем целую часть
    l2 = (lat - int(lat)) * 60;                // Берем целую часть
    lat = l1 * 100 + l2;

    l1 = int(lon);                             // Берем целую часть
    l2 = (lon - int(lon)) * 60;                // Берем целую часть
    lon = l1 * 100 + l2;

    /*
    l1 = int(lat);                             // Берем целую часть
    l2 = (lat - int(lat)) * 60;                // Берем целую часть
    l3 = (l2 - int(l2)) * 60;
    lat = l1 * 100 + int(l2) + l3/100;

    l1 = int(lon);                             // Берем целую часть
    l2 = (lon- int(lon)) * 60;                 // Берем целую часть
    l3 = (l2 - int(l2)) * 60;
    lon = l1 * 100 + int(l2) + l3/100;
    */
    const   double    koef = 0.01666666666;          ///< Коефициент перевода
    nmeaLat = QString::number(lat, 'g', 8);
    nmeaLon = QString::number(lon, 'g', 8);

    /*
    char coord [10] = {0};
    memcpy(coord, nmeaLat.toStdString().c_str(), nmeaLat.size());
    double var = atof(coord);               ///< Переменная из сообщения
    var = (var - (int)(var - (int)var % 100))
        * koef + (int)(var - (int)var % 100) / 100;

    nmeaLatitude->setText(nmeaLat) ;
    nmeaLongtitude->setText(nmeaLon);
    */
    if(nmeaLat >= 0){
     north = "N";
    }
    else {
     north = "S";
    }
    if (nmeaLon >= 0){
     east = "E";
    }
    else{
     east = "W";
    }
  }
}

void
MainWindow :: initElements(){
  _layout = new QGridLayout(this);

  convertButton = new QPushButton("Convert", this);
  connect(convertButton, &QPushButton::clicked,
          this,          &MainWindow::slotConvert);

  _zone   = new QLineEdit(this);
  utmLat  = new QLineEdit( this);
  utmLon  = new QLineEdit( this);

  QLabel *latLabel, *lonLabel, *zoneLabel;
  latLabel  = new QLabel("Latitude");
  lonLabel  = new QLabel("Longtitude");
  zoneLabel = new QLabel("Zone");

  QLabel *dec = new QLabel("Decimal:");
  QLabel *nmea = new QLabel("NMEA:");

  wgsLatitude   = new QLabel();
  wgsLongtitude = new QLabel();

  nmeaLatitude = new QLabel();
  nmeaLongtitude = new QLabel();

  _layout->addWidget(zoneLabel, 0, 0);
  _layout->addWidget(latLabel, 0, 1);
  _layout->addWidget(lonLabel, 0, 2);

  _layout->addWidget(_zone, 1, 0);
  _layout->addWidget(utmLat, 1, 1);
  _layout->addWidget(utmLon, 1, 2);

  _layout->addWidget(dec, 2, 0);
  _layout->addWidget(wgsLatitude, 2, 1);
  _layout->addWidget(wgsLongtitude, 2, 2);

  _layout->addWidget(nmea, 3, 0);
  _layout->addWidget(nmeaLatitude, 3, 1);
  _layout->addWidget(nmeaLongtitude, 3, 2);

  _layout->addWidget(&comPortNumber, 4, 0);
  _layout->addWidget(convertButton, 4, 1);
  angleLabel = new QLabel("0");
  //angleLabel->
  angleLabel->setMinimumSize(100, 100);
  _layout->addWidget(convertButton, 4, 1);
  _layout->addWidget(angleLabel, 5, 1);

  this->setLayout(_layout);
  this->show();
}

/*
void
MainWindow::paintEvent(QPaintEvent *event) {
/*    QPainter painter(this);
    painter.setPen(QPen(Qt::black, 1, Qt::DashDotLine, Qt::RoundCap));
    painter.drawLine(0, 0, 200, 200);
    */

/*
  coordPainter.setPen(QPen(Qt::black, 12, Qt::DashDotLine, Qt::RoundCap));
  coordPainter.drawLine(QLineF(10.0, 80.0, 90.0, 20.0) );
  coordPainter.drawPixmap(0, 0, 400, 400, coordPixmap);

  coordLabel.setPixmap(coordPixmap);
  coordLabel.setFixedSize(400,400);
}
*/


/*
void
MainWindow :: formRightSight(){
  QVBoxLayout *rightSightLay = new QVBoxLayout;

  imageLabel.setBackgroundRole(QPalette::Base);
  imageLabel.setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  imageLabel.setScaledContents(true);
  QImageReader imageReader(QString("%1/arrow.png")
                           .arg(QDir().absolutePath()));
  image = imageReader.read();
  if(image.colorSpace().isValid()){
    image.convertToColorSpace(QColorSpace::SRgb);
  }
  pm = QPixmap::fromImage(image);
  rm.rotate(95);
  pm = pm.transformed(rm);

  imageLabel.setPixmap(pm);
  imageLabel.setFixedSize(QSize(100,100));

  rightSightLay->addWidget(&imageLabel);
  horizontalLay->addLayout(rightSightLay);
}
*/

int
MainWindow::convertUTMtoLL(const int &s_Zone,
                               const double &s_Easting,
                               const double &s_Northing){

    int digitsLL = 6;
    /*
    QString sd_Easting  = s_Easting;
    QString sd_Northing = s_Northing;

    sd_Easting.replace( ",", "." );
    sd_Northing.replace( ",", "." );

    zone        = s_Zone.toInt();
    easting     = sd_Easting.toDouble();
    northing    = sd_Northing.toDouble();
    */

// **********************************************************************************************

    int	   err              = 0;
    //v_Ellipsoid.append( Ellipsoid( "WGS-84",                 6378137., 0.00669437999 ) ); // 22  a = 6378137 m, f = 1/298.257223563, e2 = f(2-f) = 0.00669437999

    double k0				= 0.9996;
    double a				= 6378137.;
    double eccSquared		= 0.00669437999;
    double eccPrimeSquared	= (eccSquared)/(1.-eccSquared);
    double e1				= (1.-sqrt(1.-eccSquared))/(1.+sqrt(1.-eccSquared));
    double N1, T1, C1, R1;
    double D;
    double mu;
    double phi1Rad;
//  double phi1;

    double dLatitude        = 0.;
    double dLongitude       = 0.;

    double dUTMEasting		= easting;
    double dUTMNorthing		= northing;

    double PI				= 3.14159265;
    double rad2deg			= 180./PI;

    int ZoneNumber			= -1;
    int ZoneLetter			= -1;

/*****************************************************************************/

    if ( getZone(QString::number( s_Zone), ZoneNumber, ZoneLetter ) < 0 )
        err = -1;

    if ( ( ZoneLetter < 78 ) && ( ( dUTMNorthing < 1118385 ) || ( dUTMNorthing > 9999999 ) ) )
        err = -2;

    if ( ( ZoneLetter >= 78 ) && ( ( dUTMNorthing < 0 ) || ( dUTMNorthing > 9328112 ) ) )
        err = -3;

    if ( ( dUTMEasting < 161622 ) || ( dUTMEasting > 999999 ) ) // 778266, 809999
        err = -4;

    if ( err != 0)
    {
        latitude  = "-999.999";
        longitude = "-999.999";
        return( err );
    }

/*****************************************************************************/

    dUTMEasting -= 500000.0; //remove 500,000 meter offset for longitude

    if ( ZoneLetter < 78 )
        dUTMNorthing -= 10000000.0;//remove 10,000,000 meter offset used for southern hemisphere

    mu          = (dUTMNorthing/k0)/(a*(1.-eccSquared/4.-3.*eccSquared*eccSquared/64.-5.*eccSquared*eccSquared*eccSquared/256.));
    phi1Rad     = mu+(3.*e1/2.-27.*e1*e1*e1/32.)*sin(2.*mu)+(21.*e1*e1/16.-55.*e1*e1*e1*e1/32.)*sin(4.*mu)+(151.*e1*e1*e1/96.)*sin(6.*mu);
//  phi1        = phi1Rad*rad2deg;
    N1          = a/sqrt(1.-eccSquared*sin(phi1Rad)*sin(phi1Rad) );
    T1          = tan(phi1Rad)*tan(phi1Rad);
    C1          = eccPrimeSquared*cos(phi1Rad)*cos(phi1Rad);
    R1          = a*(1.-eccSquared)/pow(1.-eccSquared*sin(phi1Rad)*sin(phi1Rad),1.5);
    D           = dUTMEasting/(N1*k0);

    dLatitude   = rad2deg*(phi1Rad-(N1*tan(phi1Rad)/R1)*(D*D/2.-(5.+3.*T1+10.*C1-4.*C1*C1-9.*eccPrimeSquared)*D*D*D*D/24.+(61.+90.*T1+298.*C1+45.*T1*T1-252.*eccPrimeSquared-3.*C1*C1)*D*D*D*D*D*D/720.));
    dLongitude	= (((double) ZoneNumber-1.)*6.-180.+3.)+rad2deg*((D-(1.+2.*T1+C1)*D*D*D/6.+(5.-2.*C1+28.*T1-3.*C1*C1+8.*eccPrimeSquared+24.*T1*T1)*D*D*D*D*D/120.)/cos(phi1Rad));

    if ( digitsLL > 0 ) {
        latitude  = QString( "%1" ).arg( dLatitude, 0, 'f', digitsLL );
        longitude = QString( "%1" ).arg( dLongitude, 0, 'f', digitsLL );
    }
    /*
    else {
        convertLLtoLL( QString( "%1" ).arg( dLatitude, 0, 'f', 7 ), QString( "%1" ).arg( dLongitude, 0, 'f', 7 ) );
    }
    */

    if ( !sepDot )
    {
        latitude.replace( ".", "," );
        longitude.replace( ".", "," );
    }

    return( err );
}

int
MainWindow::getZone( const QString &UTMZone, int &ZoneNumber, int &ZoneLetter )
{
    if ( ( UTMZone.isEmpty() == true ) || ( UTMZone == "_ERROR_" ) )
        return (-1);

    int l = UTMZone.length();

    ZoneLetter = UTMZone.at( l-1 ).toUpper().toLatin1();

    if ( ( 48 <= ZoneLetter ) && ( ZoneLetter <= 57 ) ) // ZoneLetter between 0,..,9
    {
        ZoneNumber = UTMZone.left( l ).toInt();
        ZoneLetter = 78;
    }
    else
        ZoneNumber = UTMZone.left( l-1 ).toInt();

    if ( ( ZoneLetter == 73 ) || ( ZoneLetter < 67 ) )
        ZoneLetter = 74;

    if ( ( ZoneLetter == 79 ) || ( ZoneLetter > 88 ) )
        ZoneLetter = 78;

    if ( ( ZoneNumber < 1 ) || ( ZoneNumber > 60 ) )
        return (-2);

    return( 0 );
}

void
MainWindow :: keyPressEvent(QKeyEvent *event){
  int key = event->key();
  if(key == Qt::Key_Left || key == Qt::Key_Down){
    angle -= 1;
    if(angle < 0)
      angle = 359;
    angleLabel->setText(QString::number(angle));
  }

  if(key == Qt::Key_Right || key == Qt::Key_Up){
    angle+= 1;
    if(angle > 359)
      angle = 0;
    angleLabel->setText(QString::number(angle));
  }
}
/*!
 * Парсим GGA-сообщение
 *GGA Global Positioning System Fix Data. Time, Position and fix related data for a GPS receiver
 * $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M ,  ,*47
 *    0     1      2      3    4      5 6  7  8   9    10 11  12 13  14
 *          Time   Lat         Lon
 *         1         2    3    4     5 6  7  8   9  10 11 12 13   14 15
 *         |         |    |    |     | |  |  |   |  |  |  |  |    |  |
 *$--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx*hh
 *1) Time (UTC)
 *2) Latitude
 *3) N or S (North or South)
 *4) Longitude
 *5) E or W (East or West)
 *6) GPS Quality Indicator,
 *    0 - fix not available,
 *    1 - GPS fix,
 *    2 - Differential GPS fix
 *7) Number of satellites in view, 00 - 12
 *8) Horizontal Dilution of precision
 *9) Antenna Altitude above/below mean-sea-level (geoid)
 *10) Units of antenna altitude, meters
 *11) Geoidal separation, the difference between the WGS-84 earth
 *    ellipsoid and mean-sea-level (geoid), "-" means mean-sea-level below ellipsoid
 *12) Units of geoidal separation, meters
 *13) Age of differential GPS data, time in seconds since last SC104
 *    type 1 or 9 update, null field when DGPS is not used
 *14) Differential reference station ID, 0000-1023
 *15) Checksum
 */
void
MainWindow :: sendNmea(){
  QString nmeaSentence = QString("$GPGGA,123519,%1,%2,%3,%4,1,08,0.9,545.4,M,46.9,M ,  ,*47")
                          .arg(nmeaLat)
                          .arg(north)
                          .arg(nmeaLon)
                          .arg(east);


}
