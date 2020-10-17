#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
                      : QWidget(parent) {
  initElements();


  chart.addSeries(&ls);
  chart.legend()->hide();

  chart.addAxis(&x_axe, Qt::AlignBottom);
  chart.addAxis(&y_axe, Qt::AlignLeft);

  ls.attachAxis(&x_axe);
  ls.attachAxis(&y_axe);

  chartView.setChart(&chart);
  chartView.setRenderHint(QPainter::Antialiasing);
  mainLayout.addWidget(&chartView);

  connected = true;

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
    connect(&ping,  &QTimer     ::  timeout,
            this,   &MainWindow ::  slotTimerPing);
    ping.start(100);
  }
}


/// Обработчик вызова таймера.
void
MainWindow::slotTimerPing(){
  static quint8 count = 0;

  double rad          = qDegreesToRadians((double)angleSB.value());
  double sin_easting  = sin(qDegreesToRadians((double)angleSB.value()));
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

  //qint64 bytes = m_serial->write(QByteArray().append(nmeaSentence));
  ls.append(northing, easting);
  if(!axesSett){
    x_axe.setMin(northing - 50);
    x_axe.setMax(northing + 50);
    y_axe.setMin(easting - 50);
    y_axe.setMax(easting + 50);
    axesSett = true;
  }
  getLastMeasMinMax();
  //qDebug() << "Tanslated " << bytes << " bytes.";
  /*
  if(bytes > 0){
    qDebug() << nmeaSentence;
  }
  */
}

void
MainWindow :: changeBorders(qreal minX,
                            qreal maxX,
                            qreal minY,
                            qreal maxY){
    qlonglong newMinX = minX < x_axe.min() ? minX : x_axe.min();
    qlonglong newMaxX = maxX > x_axe.max() ? maxX : x_axe.max();
    qlonglong newMinY = minY < y_axe.min() ? minY : y_axe.min();
    qlonglong newMaxY = maxY > y_axe.max() ? maxY : y_axe.max();

    quint8 scaleVal = 50;
    newMaxX += newMaxX % scaleVal
             ? scaleVal - newMaxX % scaleVal
             : 0;
    newMaxY += newMaxY % scaleVal
             ? scaleVal - newMaxY % scaleVal
             : 0;

    newMinX -= newMinX % scaleVal
             ? scaleVal + newMinX % scaleVal
             : 0;
    newMinY -= newMinY % scaleVal
             ? scaleVal + newMinY % scaleVal
             : 0;

    x_axe.setMin(newMinX);
    x_axe.setMax(newMaxX);
    y_axe.setMin(newMinY);
    y_axe.setMax(newMaxY);
}

void
MainWindow :: getLastMeasMinMax(){
  static QList <qreal> tempXVal;
  static QList <qreal> tempYVal;
  static quint8 counter = 0;
  if(!(++counter % 10)){
    counter = 0;
    changeBorders(getMinVal(tempXVal),
                  getMaxVal(tempXVal),
                  getMinVal(tempYVal),
                  getMaxVal(tempYVal));
    tempXVal.clear();
    tempYVal.clear();
  }
  tempXVal.append(ls.at(ls.count() - 1).x());
  tempYVal.append(ls.at(ls.count() - 1).y());
}

qreal
MainWindow :: getMinVal(const QList <qreal> &list){
  if(list.isEmpty())
    return -1;

  qreal min = list.at(0);

  foreach (qreal val, list) {
    if(val < min)
      min = val;
  }
  return  min;
}

qreal
MainWindow :: getMaxVal(const QList <qreal> &list){
  if(list.isEmpty())
    return -1;
  qreal max = list.at(0);
  foreach(qreal val, list){
    if(val > max)
      max = val;
  }
  return max;
}


/// Инициализация графических объектов.
void
MainWindow :: initElements(){
  setGuiText();
  setGuiPlacement();
  setDefParams();
  setConnections();
  mainLayout.addLayout(&layout);

  this->setLayout(&mainLayout);
  this->resize(1000,500);
  this->show();
}

void
MainWindow ::  setGuiText(){
  LatitudeLbl   .setText("Довгота, верт");
  LongtitudeLbl .setText("Широта, гориз");
  ZoneLbl       .setText("Zone");
  DecimalLbl    .setText("Decimal:");
  NMEALbl       .setText("NMEA:");
  comNumLbl     .setText("COM №:");
  angleLbl      .setText("Angle:");
  startB        .setText("Start");             ///< Кнопка запуска работы программы.
}


void
MainWindow :: setGuiPlacement(){
  layout.addWidget(&ZoneLbl,         0, 0);
  layout.addWidget(&LatitudeLbl,     0, 1);
  layout.addWidget(&LongtitudeLbl,   0, 2);
  layout.addWidget(&zoneLE,          1, 0);
  layout.addWidget(&utmLatLE,        1, 1);
  layout.addWidget(&utmLonLE,        1, 2);
  layout.addWidget(&DecimalLbl,      2, 0);
  layout.addWidget(&wgsLatitudeL,    2, 1);
  layout.addWidget(&wgsLongtitudeL,  2, 2);
  layout.addWidget(&NMEALbl,         3, 0);
  layout.addWidget(&nmeaLatitudeL,   3, 1);
  layout.addWidget(&nmeaLongtitudeL, 3, 2);
  layout.addWidget(&comNumLbl,       4, 0);
  layout.addWidget(&comPortNumberSB, 4, 1);
  layout.addWidget(&angleLbl,        5, 0);
  layout.addWidget(&angleSB,         5, 1);
  layout.addWidget(&startB,          6, 1);
}

void
MainWindow :: setDefParams(){
  //-- Некоторые значения для выбора угла движения.
  angleSB.setRange(0, 360 - 1);
  angleSB.setWrapping(true);
  angleSB.setValue(180);
}


void
MainWindow ::  setConnections(){
  connect(&startB,    &QPushButton  ::  clicked,
          this,       &MainWindow   ::  startButton);
}

/// Порядок подключения к COM порту.
void
MainWindow ::  connectToComPort(){
  m_serial = new QSerialPort;
  QString portName = QString("/dev/ttyUSB%1").arg(comPortNumberSB.value());

  m_serial->setPortName(portName);
  m_serial->setBaudRate   (QSerialPort::Baud9600);
  m_serial->setDataBits   (QSerialPort::Data8);
  m_serial->setParity     (QSerialPort::EvenParity);
  m_serial->setStopBits   (QSerialPort::OneStop);
  m_serial->setFlowControl(QSerialPort::NoFlowControl);

  if(m_serial->open(QIODevice::ReadWrite)){
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

    int length = UTMZone.length();

    ZoneLetter = UTMZone.at(length - 1).toUpper().toLatin1();

    if ( ( 48 <= ZoneLetter ) && ( ZoneLetter <= 57 ) ) // ZoneLetter between 0,..,9
    {
        ZoneNumber = UTMZone.left(length).toInt();
        ZoneLetter = 78;
    }
    else{
        ZoneNumber = UTMZone.left(length - 1).toInt();
    }

    if ( ( ZoneLetter == 73 ) || ( ZoneLetter < 67 ) ){
        ZoneLetter = 74;
    }
    if ( ( ZoneLetter == 79 ) || ( ZoneLetter > 88 ) ){
        ZoneLetter = 78;
    }

    if ( ( ZoneNumber < 1 ) || ( ZoneNumber > 60 ) ) {
        return (-2);
    }

    return( 0 );
}

