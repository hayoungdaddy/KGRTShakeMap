#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    recvEEWMessage = new RecvMessage(this);
    connect(recvEEWMessage, SIGNAL(sendEEWMessageToMainWindow(_BINARY_EEW_PACKET)), this, SLOT(rvEEWMessageFromThread(_BINARY_EEW_PACKET)));
    if(!recvEEWMessage->isRunning())
    {
        recvEEWMessage->setup(QUrl("ws://10.65.0.60:30900"));
        recvEEWMessage->start();
    }

    recvPOINTMessage = new RecvMessage(this);
    connect(recvPOINTMessage, SIGNAL(sendPOINTSMessageToMainWindow(_BINARY_POINT_PACKET)),
            this, SLOT(rvPOINTSMessageFromThread(_BINARY_POINT_PACKET)));
    connect(recvPOINTMessage, SIGNAL(sendSTATIONMessageToMainWindow(_BINARY_STATION_PACKET)),
            this, SLOT(rvSTATIONMessageFromThread(_BINARY_STATION_PACKET)));
    if(!recvPOINTMessage->isRunning())
    {
        recvPOINTMessage->setup(QUrl("ws://10.65.0.3:30920"));
        recvPOINTMessage->start();
    }

    ui->eventListCB->setEnabled(false);

    systemTimer = new QTimer;
    connect(systemTimer, SIGNAL(timeout()), this, SLOT(doRepeatWork()));

    connect(ui->stopPB, SIGNAL(clicked()), this, SLOT(stopPBClicked()));
    connect(ui->playPB, SIGNAL(clicked()), this, SLOT(playPBClicked()));
    connect(ui->currentPB, SIGNAL(clicked()), this, SLOT(currentPBClicked()));

    currentPBClicked();

    connect(ui->dateDial, SIGNAL(valueChanged(int)), this, SLOT(dDialChanged(int)));
    connect(ui->hourDial, SIGNAL(valueChanged(int)), this, SLOT(hDialChanged(int)));
    connect(ui->minDial, SIGNAL(valueChanged(int)), this, SLOT(mDialChanged(int)));
    connect(ui->secDial, SIGNAL(valueChanged(int)), this, SLOT(sDialChanged(int)));

    native = new Widget(&mypainter, this);
    ui->mapLO->addWidget(native);

    dataSrc = "KISS 100 Samples, Accelaration";
    native->setDataSrc(dataSrc);

    connect(ui->eventListCB, SIGNAL(currentIndexChanged(int)), this, SLOT(eventListCBChanged(int)));

    monChanID = 3;
    monChanCBChanged(3);
    connect(ui->monChanCB, SIGNAL(currentIndexChanged(int)), this, SLOT(monChanCBChanged(int)));


    dataTypeBG = new QButtonGroup(this);
    dataTypeBG->addButton(ui->sTypeRB); dataTypeBG->setId(ui->sTypeRB, 0);
    dataTypeBG->addButton(ui->cTypeRB); dataTypeBG->setId(ui->cTypeRB, 1);

    legendTypeBG = new QButtonGroup(this);
    legendTypeBG->addButton(ui->kigamLegendRB); legendTypeBG->setId(ui->kigamLegendRB, 0);
    legendTypeBG->addButton(ui->kmaLegendRB); legendTypeBG->setId(ui->kmaLegendRB, 1);

    ui->sTypeRB->setChecked(true);
    ui->kigamLegendRB->setChecked(true);

    connect(ui->sTypeRB, SIGNAL(clicked()), this, SLOT(dataTypeChanged()));
    connect(ui->cTypeRB, SIGNAL(clicked()), this, SLOT(dataTypeChanged()));

    dataType = 0;
    dataTypeChanged();

    connect(ui->kigamLegendRB, SIGNAL(clicked()), this, SLOT(legendTypeChanged()));
    connect(ui->kmaLegendRB, SIGNAL(clicked()), this, SLOT(legendTypeChanged()));

    legendType = 0;
    legendTypeChanged();
}

void MainWindow::legendTypeChanged()
{
    if(ui->kigamLegendRB->isChecked())
        legendType = 0;

    if(ui->kmaLegendRB->isChecked())
        legendType = 1;

    native->setLegendType(legendType);

    bool valid = timeCheck(dataTimeUTC);
    if(valid)
    {
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));
    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_STATION_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

void MainWindow::dataTypeChanged()
{
    if(ui->sTypeRB->isChecked())
        dataType = 0;

    if(ui->cTypeRB->isChecked())
        dataType = 1;

    native->setTypeID(dataType);
    recvPOINTMessage->setDataType(dataType);

    bool valid = timeCheck(dataTimeUTC);
    if(valid)
    {
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));
    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_STATION_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

void MainWindow::monChanCBChanged(int chanIndex)
{
    monChanID = chanIndex;
    native->setChanID(monChanID);
    recvPOINTMessage->setChanID(monChanID);

    bool valid = timeCheck(dataTimeUTC);
    if(valid)
    {
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));
    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_STATION_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::stopPBClicked()
{
    systemTimer->stop();
    isNowPlayMode = false;
    isStopMode = true;

    dDialChanged(ui->dateDial->value());
    hDialChanged(ui->hourDial->value());
    mDialChanged(ui->minDial->value());
    sDialChanged(ui->secDial->value());

    ui->playPB->setEnabled(true);
    ui->stopPB->setEnabled(false);
    ui->currentPB->setEnabled(true);
    ui->dateDial->setEnabled(true);
    ui->hourDial->setEnabled(true);
    ui->minDial->setEnabled(true);
    ui->secDial->setEnabled(true);

    if(eewpacket.numEVENT != 0)
        ui->eventListCB->setEnabled(true);
    else
        ui->eventListCB->setEnabled(false);
}

void MainWindow::playPBClicked()
{
    isStopMode = false;
    ui->dateLB->setText("DATE");
    ui->hourLB->setText("HOUR");
    ui->minLB->setText("MINUTE");
    ui->secLB->setText("SECOND");

    systemTimer->start(1000);

    ui->playPB->setEnabled(false);
    ui->stopPB->setEnabled(true);
    ui->currentPB->setEnabled(false);
    ui->dateDial->setEnabled(false);
    ui->hourDial->setEnabled(false);
    ui->minDial->setEnabled(false);
    ui->secDial->setEnabled(false);
    ui->eventListCB->setEnabled(false);
}

void MainWindow::currentPBClicked()
{
    isNowPlayMode = true;
    ui->dateDial->setValue(1);
    playPBClicked();
}

void MainWindow::setDialAndLCD(QDateTime dtUTC)
{
    ui->dataTimeLCD->display(dtUTC.toString("yy-MM-dd hh:mm:ss"));

    QDateTime nowUTC = QDateTime::currentDateTimeUtc();

    if(dtUTC.date() == nowUTC.date())
        ui->dateDial->setValue(1);
    else
        ui->dateDial->setValue(0);
    ui->hourDial->setValue(dtUTC.time().hour());
    ui->minDial->setValue(dtUTC.time().minute());
    ui->secDial->setValue(dtUTC.time().second());
}

QDateTime MainWindow::findDataTimeUTC()
{
    QDateTime dtUTC;
    dtUTC.setTimeSpec(Qt::UTC);
    if(ui->dateDial->value() == 1)
        dtUTC = QDateTime::currentDateTimeUtc();
    else
        dtUTC = QDateTime::currentDateTimeUtc().addDays(-1);

    int hour = ui->hourDial->value();
    int min = ui->minDial->value();
    int sec = ui->secDial->value();

    QTime t; t.setHMS(hour, min, sec);
    dtUTC.setTime(t);

    return dtUTC;
}

void MainWindow::dDialChanged(int date)
{
    if(isNowPlayMode)
        return;

    QDateTime nowUTC;

    if(date == 1)
        nowUTC = QDateTime::currentDateTimeUtc();
    else
        nowUTC = QDateTime::currentDateTimeUtc().addDays(-1);

    if(isStopMode)
        ui->dateLB->setText(nowUTC.toString("MM-dd"));
}

void MainWindow::hDialChanged(int hour)
{
    if(isNowPlayMode)
        return;

    if(isStopMode)
        ui->hourLB->setText(QString::number(hour));
}

void MainWindow::mDialChanged(int min)
{
    if(isNowPlayMode)
        return;

    if(isStopMode)
        ui->minLB->setText(QString::number(min));
}

void MainWindow::sDialChanged(int sec)
{
    if(isNowPlayMode)
        return;

    if(isStopMode)
        ui->secLB->setText(QString::number(sec));
}

void MainWindow::eventListCBChanged(int eventIndex)
{
    if(eventIndex == 0 || eventIndex == -1)
        return;

    QDateTime eventtime = QDateTime::fromString(ui->eventListCB->currentText().section(" M", 0, 0),
                                                "yyyy-MM-dd hh:mm:ss");

    QDateTime today = QDateTime::currentDateTimeUtc();
    if(eventtime.date() == today.date())
        ui->dateDial->setValue(1);
    else
        ui->dateDial->setValue(0);

    eventtime = eventtime.addSecs(-2);

    ui->hourDial->setValue(eventtime.time().hour());
    ui->minDial->setValue(eventtime.time().minute());
    ui->secDial->setValue(eventtime.time().second());
}

void MainWindow::doRepeatWork()
{
    if(isNowPlayMode)
    {
        QDateTime systemTimeUTC = QDateTime::currentDateTimeUtc();
        dataTimeUTC = systemTimeUTC.addSecs(- SECNODS_FOR_ALIGN_QSCD); // GMT  
    }
    else
    {
        dataTimeUTC = findDataTimeUTC();
        dataTimeUTC = dataTimeUTC.addSecs(1);
    }

    setDialAndLCD(dataTimeUTC);

    bool valid = timeCheck(dataTimeUTC);
    if(valid)
    {
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));
    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_STATION_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

bool MainWindow::timeCheck(QDateTime dt)
{
    QDateTime now = QDateTime::currentDateTimeUtc();

    if(dt > now)
        return false;
    else
    {
        int diff = now.toTime_t() - dt.toTime_t();
        if(diff > DATA_DURATION)
            return false;
        else
            return true;
    }
}

void MainWindow::rvEEWMessageFromThread(_BINARY_EEW_PACKET packet)
{
    eewpacket = packet;

    ui->eventListCB->clear();

    QStringList events;
    events << "Select a Available EEW Event";

    if(eewpacket.numEVENT != 0)
    {
        for(int i=eewpacket.numEVENT-1;i>=0;i--)
        {
            QDateTime et;
            et.setTimeSpec(Qt::UTC);
            et.setTime_t(eewpacket.eventlist[i].eventEpochStartTime);

            QString eventName = et.toString("yyyy-MM-dd hh:mm:ss") + " M" + QString::number(eewpacket.eventlist[i].mag, 'f', 1);
            events << eventName;
        }
    }

    ui->eventListCB->addItems(events);
}

void MainWindow::rvPOINTSMessageFromThread(_BINARY_POINT_PACKET pointpacket)
{
    _BINARY_STATION_PACKET stationpacket;
    stationpacket.dataTime = 0;
    native->animate(eewpacket, pointpacket, stationpacket);
}

void MainWindow::rvSTATIONMessageFromThread(_BINARY_STATION_PACKET stationpacket)
{
    _BINARY_POINT_PACKET pointpacket;
    pointpacket.dataTime = 0;
    native->animate(eewpacket, pointpacket, stationpacket);
}
