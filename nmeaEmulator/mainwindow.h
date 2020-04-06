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

#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QKeyEvent>
#include <QTimer>
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
  //---------------------------------
  /*
  QHBoxLayout *horizontalLay;

  QPainter coordPainter;
  QPixmap coordPixmap;
  QLabel coordLabel;

  QImage image;
  QLabel imageLabel;
  QPixmap pm;
  QMatrix rm;
  */
  //-----------------------------------------------------
  QPushButton *convertButton;
  QLineEdit *_zone, *utmLat, *utmLon;
  QLabel *angleLabel;

  QLabel  *wgsLatitude, *wgsLongtitude;
  QLabel  *nmeaLatitude, *nmeaLongtitude;
  QGridLayout *_layout;

  QString latitude;   ///< resultOfConvertation
  QString longitude;  ///< resultOfConvertation
  QString nmeaLat, nmeaLon;
  QString north, east;

  double angle = 0;
  int zone = 36;
  double  easting = 298849.0;
  double  northing = 5536044.0;
  int distanse = 3;
  QTimer ping;


  ~MainWindow();

public slots:
    void slotConvert(bool clicked);
    void slotTimerPing();

private:
  QSerialPort *m_serial = nullptr;
  void initElements();
  void formRightSight();
  int convertUTMtoLL(const QString &s_Zone,
                     const QString &s_Easting,
                     const QString &s_Northing);
  int getZone( const QString &UTMZone, int &ZoneNumber, int &ZoneLetter );
  bool sepDot = true;
  void sendNmea();

protected:
//    void paintEvent(QPaintEvent *event);
  void keyPressEvent(QKeyEvent *event);
};










#endif // MAINWINDOW_H
