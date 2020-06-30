#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDebug>

#include <QApplication>
#include <QWidget>
#include <QObject>

#include <qmath.h>

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QGridLayout>
#include <QHBoxLayout>

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QKeyEvent>
#include <QTimer>
#include <QDateTime>

#include <locale.h>     //setLocale
#include <stdlib.h>     //atof
/*
#include <QImageReader>
#include <QColorSpace>
#include <QDir>
#include <QImage>
#include <QPainter>


*/


class MainWindow : public QWidget{
  Q_OBJECT
public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();


public slots:
    /// Действие после нажатия кнопи старт.
    void startButton(bool clicked);


    /// Обработчик вызова таймера.
    void slotTimerPing();

private:
  QGraphicsView*  view;
  QGraphicsScene* scene;

  QTimer ping;                      ///< Таймер для генерации новой строки NMEA.
  QSerialPort *m_serial = nullptr;  ///< Объект COM порта.

  bool connected = false;           ///< Флаг соединения.

  QLineEdit zoneLE,                 ///< Значение Зоны (UTM).
            utmLatLE,               ///< Значение Широты (UTM).
            utmLonLE;               ///< Значение Долготы(UTM).

  QLabel    wgsLatitudeL,           ///< Широта (decimal).  horizontal
            wgsLongtitudeL,         ///< Долгота (decimal). vertical
            nmeaLatitudeL,          ///< Широта (NMEA).
            nmeaLongtitudeL;        ///< Долгота (NMEA).

  QSpinBox  comPortNumberSB,        ///< Номер COM порта.
            angleSB;                ///< Угол движения.


  QString latitude;   ///< resultOfConvertation
  QString longitude;  ///< resultOfConvertation
  QString nmeaLat, nmeaLon;
  QString north, east;

  int zone = 36;
  double  easting = 298849.0;       ///< Широта
  double  northing = 5536044.0;     ///< Долгота
  const double distanse = 1.0;            ///< Изменение координат.


  /// Инициализация графических объектов.
  void initElements();

  /// Порядок подключения к COM порту.
  void connectToComPort();

  void convertData(bool clicked);

  /// Конвертация координат из UTM в WGS
  int convertUTMtoLL();

  /// Статус вычисления UTM зоны.
  int getZone( const QString &UTMZone,
               int &ZoneNumber,
               int &ZoneLetter );
  bool sepDot = true;

};










#endif // MAINWINDOW_H
