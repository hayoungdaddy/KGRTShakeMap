#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWebSockets/QtWebSockets>
#include <QButtonGroup>

#include <QTimer>

#include "common.h"
#include "recvmessage.h"

#include "Painter.h"
#include "widget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    RecvMessage *recvPOINTMessage;
    RecvMessage *recvEEWMessage;

    QTimer *systemTimer;
    QDateTime dataTimeUTC;

    _BINARY_EEW_PACKET eewpacket;
    _BINARY_POINT_PACKET pointpacket;

    bool isNowPlayMode;
    bool isStopMode;
    bool timeCheck(QDateTime);

    Painter mypainter;
    Widget *native;

    QString dataSrc;
    int monChanID;
    int dataType;
    int legendType;

    QButtonGroup *dataTypeBG, *legendTypeBG;

private slots:
    void rvEEWMessageFromThread(_BINARY_EEW_PACKET);
    void rvPOINTSMessageFromThread(_BINARY_POINT_PACKET);
    void rvSTATIONMessageFromThread(_BINARY_STATION_PACKET);

    void doRepeatWork();
    void stopPBClicked();
    void playPBClicked();
    void currentPBClicked();
    void setDialAndLCD(QDateTime);
    QDateTime findDataTimeUTC();

    void dDialChanged(int);
    void hDialChanged(int);
    void mDialChanged(int);
    void sDialChanged(int);

    void eventListCBChanged(int);
    void monChanCBChanged(int);

    void dataTypeChanged();
    void legendTypeChanged();
};
#endif // MAINWINDOW_H
