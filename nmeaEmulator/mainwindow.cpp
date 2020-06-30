#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
                      : QWidget(parent) {
  initElements();
}

MainWindow::~MainWindow() {

}


/// Действие после нажатия кнопи старт.
void
MainWindow :: startButton(bool clicked) {
  //-- Если не подключены к СОМ пору то подключаемся.
  if(!connected){
    connectToComPort();
  }
  //-- Если подключены
  if(connected){
    //-- Устанавливаем вызов для таймера с периодом времени.
    connect(&ping,  &QTimer::timeout,
            this,   &MainWindow::slotTimerPing);
    ping.setInterval(600);
  }
}


/// Обработчик вызова таймера.
void
MainWindow::slotTimerPing(){
  static quint8 count = 0;

  double rad = qDegreesToRadians((double)angleSB.value());
  double sin_easting = sin(qDegreesToRadians((double)angleSB.value()));
  double cos_northing = cos(qDegreesToRadians((double)angleSB.value()));
  easting   = easting  + sin(qDegreesToRadians((double)angleSB.value())) * distanse;
  northing  = northing + cos(qDegreesToRadians((double)angleSB.value())) * distanse;

  convertData(false);
  QString nmeaSentence;
  count++;
  switch (count % 4) {
  case 0: {
    nmeaSentence = QString("$GPGGA,123519,%1,%2,%3,%4,1,08,0.9,545.4,M,46.9,*47\n")
                        .arg(nmeaLat) .arg(north) .arg(nmeaLon) .arg(east);
    break;
  }

  case 1: {
    nmeaSentence = QString("$GPVTG,t,T,,,s.ss,N,s.ss,K*hh\n");
    break;
  }

  case 2: {
    nmeaSentence = QString("$GPRMC,%1,A,%2,%3,%4,%5,000.0,%6.0,%7,020.3,E*68\n")
                    .arg(QDateTime::currentDateTime().toString("hhmmss"))
                    .arg(nmeaLat) .arg(north) .arg(nmeaLon) .arg(east)
                    .arg(angleSB.value())
                    .arg(QDateTime::currentDateTime().toString("ddMMyy"));
    break;
  }

  case 3: {
    nmeaSentence = QString("$GPGLL,%1,%2,%3,%4*75\n")
                    .arg(nmeaLat) .arg(north) .arg(nmeaLon) .arg(east);
    break;
  }

  default:
    nmeaSentence = QString("$GPGGA,123519,%1,%2,%3,%4,1,08,0.9,545.4,M,46.9,*47\n")
                        .arg(nmeaLat) .arg(north) .arg(nmeaLon) .arg(east);
  }

  qint64 bytes = m_serial->write(QByteArray().append(nmeaSentence));
  qDebug() << "Tanslated " << bytes << " bytes.";
  if(bytes > 0){
    qDebug() << nmeaSentence;
  }
}


/// Инициализация графических объектов.
void
MainWindow :: initElements(){

  //-- Некоторые значения для выбора угла движения.
  angleSB.setRange(0, 360 - 1);
  angleSB.setWrapping(true);
  angleSB.setValue(180);


  // Слой.
  QHBoxLayout* hLayout = new QHBoxLayout();
  QGridLayout* layout = new QGridLayout();

  QPushButton* startB = new QPushButton("Start", this);             ///< Кнопка запуска работы программы.
  connect(startB, &QPushButton::clicked,
          this,   &MainWindow::startButton);

  QLabel* LatitudeLbl   = new QLabel("Довгота, верт");
  QLabel* LongtitudeLbl = new QLabel("Широта, гориз");
  QLabel* ZoneLbl       = new QLabel("Zone");
  QLabel* DecimalLbl    = new QLabel("Decimal:");
  QLabel* NMEALbl       = new QLabel("NMEA:");
  QLabel* comNumLbl     = new QLabel("COM №:");
  QLabel* angleLbl      = new QLabel("Angle:");


  layout->addWidget(ZoneLbl, 0, 0);
  layout->addWidget(LatitudeLbl, 0, 1);
  layout->addWidget(LongtitudeLbl, 0, 2);

  layout->addWidget(&zoneLE, 1, 0);
  layout->addWidget(&utmLatLE, 1, 1);
  layout->addWidget(&utmLonLE, 1, 2);

  layout->addWidget(DecimalLbl, 2, 0);
  layout->addWidget(&wgsLatitudeL, 2, 1);
  layout->addWidget(&wgsLongtitudeL, 2, 2);

  layout->addWidget(NMEALbl, 3, 0);
  layout->addWidget(&nmeaLatitudeL, 3, 1);
  layout->addWidget(&nmeaLongtitudeL, 3, 2);

  layout->addWidget(comNumLbl, 4, 0);
  layout->addWidget(&comPortNumberSB, 4, 1);

  layout->addWidget(angleLbl, 5, 0);
  layout->addWidget(&angleSB, 5, 1);

  layout->addWidget(startB, 6, 1);

  hLayout->addLayout(layout);


  view = new QGraphicsView;
  scene = new QGraphicsScene(view);
  QPen pen(Qt::green);
  QPainter *k =  new QPainter;

  k->drawPoint(20,20);

  scene->addLine(0, 90, 180, 90, pen);
  scene->addLine(90, 0, 90, 180, pen);
  scene->addEllipse(20.0, 20.0, 1, 1, pen);
  view->setScene(scene);
  hLayout->addWidget(view);


  this->setLayout(hLayout);
  this->show();
}


/// Порядок подключения к COM порту.
void
MainWindow ::  connectToComPort(){
  m_serial = new QSerialPort;
  QString portName = QString("/dev/ttyUSB%1").arg(comPortNumberSB.value());
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
MainWindow :: convertData(bool clicked){

  zoneLE.setText(QString::number(zone));
  utmLatLE.setText(QString::number(easting, 'g', 8));
  utmLonLE.setText(QString::number(northing, 'g', 8));

  if(convertUTMtoLL() == 0){
    wgsLatitudeL.setText(latitude);
    wgsLongtitudeL.setText(longitude);

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
    nmeaLat = QString::number(lat, 'g', 8);
    nmeaLon = QString::number(lon, 'g', 8);

    nmeaLatitudeL.setText(nmeaLat);
    nmeaLongtitudeL.setText(nmeaLon);
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



/// Конвертация координат из UTM в WGS
int
MainWindow::convertUTMtoLL(){

    int digitsLL = 6;
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

    if ( getZone(QString::number( zone), ZoneNumber, ZoneLetter ) < 0 )
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

/// Статус вычисления UTM зоны.
int
MainWindow::getZone( const QString &UTMZone, int &ZoneNumber, int &ZoneLetter ) {
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

