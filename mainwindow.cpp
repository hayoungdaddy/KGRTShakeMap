#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    recvTIMEMessage = new RecvWSMessage(this);
    connect(recvTIMEMessage, SIGNAL(sendTIMEMessageToMainWindow(int)), this, SLOT(rvTIMEMessageFromThread(int)));
    if(!recvTIMEMessage->isRunning())
    {
        recvTIMEMessage->setup(QUrl("ws://10.65.0.60:30900"));
        recvTIMEMessage->start();
    }

    recvEEWMessage = new RecvWSMessage(this);
    connect(recvEEWMessage, SIGNAL(sendEEWMessageToMainWindow(_BINARY_SMALL_EEWLIST_PACKET)), this, SLOT(rvEEWMessageFromThread(_BINARY_SMALL_EEWLIST_PACKET)));
    if(!recvEEWMessage->isRunning())
    {
        recvEEWMessage->setup(QUrl("ws://10.65.0.60:30910"));
        recvEEWMessage->start();
    }

    recvPOINTMessage = new RecvWSMessage(this);
    connect(recvPOINTMessage, SIGNAL(sendPOINTSMessageToMainWindow(_BINARY_POINT_PACKET)),
            this, SLOT(rvPOINTSMessageFromThread(_BINARY_POINT_PACKET)));
    connect(recvPOINTMessage, SIGNAL(sendSTATIONMessageToMainWindow(_BINARY_PGA_PACKET)),
            this, SLOT(rvSTATIONMessageFromThread(_BINARY_PGA_PACKET)));
    if(!recvPOINTMessage->isRunning())
    {
        recvPOINTMessage->setup(QUrl("ws://10.65.0.60:30970"));
        recvPOINTMessage->start();
    }

    ui->eventListCB->setEnabled(false);

    systemTimer = new QTimer;
    connect(systemTimer, SIGNAL(timeout()), this, SLOT(doRepeatWork()));

    connect(ui->stopPB, SIGNAL(clicked()), this, SLOT(stopPBClicked()));
    connect(ui->playPB, SIGNAL(clicked()), this, SLOT(playPBClicked()));
    connect(ui->realtimePB, SIGNAL(clicked()), this, SLOT(realtimePBClicked()));

    realtimePBClicked();

    connect(ui->dateDial, SIGNAL(valueChanged(int)), this, SLOT(dDialChanged(int)));
    connect(ui->hourDial, SIGNAL(valueChanged(int)), this, SLOT(hDialChanged(int)));
    connect(ui->minDial, SIGNAL(valueChanged(int)), this, SLOT(mDialChanged(int)));
    connect(ui->secDial, SIGNAL(valueChanged(int)), this, SLOT(sDialChanged(int)));

    native = new Widget(&mypainter, this);
    ui->mapLO->addWidget(native);

    dataSrc = "Accelaration, 100 Samples for Second";
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

    serverTimeUtc.setTimeSpec(Qt::UTC);
}

void MainWindow::legendTypeChanged()
{
    if(ui->kigamLegendRB->isChecked())
        legendType = 0;

    if(ui->kmaLegendRB->isChecked())
        legendType = 1;

    native->setLegendType(legendType);

    bool valid = timeCheck(dataTimeKST);
    if(valid)
    {
        QDateTime dataTimeUTC = convertUTC(dataTimeKST);
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));

    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_PGA_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        stationpacket.numStation = 0;
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

    bool valid = timeCheck(dataTimeKST);
    if(valid)
    {
        QDateTime dataTimeUTC = convertUTC(dataTimeKST);
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));

    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_PGA_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        stationpacket.numStation = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

void MainWindow::monChanCBChanged(int chanIndex)
{
    monChanID = chanIndex;
    native->setChanID(monChanID);
    recvPOINTMessage->setChanID(monChanID);

    bool valid = timeCheck(dataTimeKST);
    if(valid)
    {
        QDateTime dataTimeUTC = convertUTC(dataTimeKST);
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));

    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_PGA_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        stationpacket.numStation = 0;
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
    isRealTimeMode = false;
    isStopMode = true;

    QString stylesheet("background-color: rgb(238, 238, 236);");
    ui->realtimePB->setStyleSheet(stylesheet);
    ui->playPB->setStyleSheet(stylesheet);

    QString stylesheet2("background-color: rgb(52, 101, 164);");
    ui->stopPB->setStyleSheet(stylesheet2);

    dDialChanged(ui->dateDial->value());
    hDialChanged(ui->hourDial->value());
    mDialChanged(ui->minDial->value());
    sDialChanged(ui->secDial->value());

    ui->playPB->setEnabled(true);
    ui->stopPB->setEnabled(false);
    ui->realtimePB->setEnabled(true);
    ui->dateDial->setEnabled(true);
    ui->hourDial->setEnabled(true);
    ui->minDial->setEnabled(true);
    ui->secDial->setEnabled(true);

    if(eewpacket.numEEW != 0)
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
    ui->realtimePB->setEnabled(false);
    ui->dateDial->setEnabled(false);
    ui->hourDial->setEnabled(false);
    ui->minDial->setEnabled(false);
    ui->secDial->setEnabled(false);
    ui->eventListCB->setEnabled(false);
}

void MainWindow::realtimePBClicked()
{
    isRealTimeMode = true;
    ui->dateDial->setValue(1);
    playPBClicked();
}

void MainWindow::setDialAndLCDUsingKST(QDateTime dtKST)
{
    ui->dataTimeLCD->display(dtKST.toString("yy-MM-dd hh:mm:ss"));

    QDateTime nowUTC = QDateTime::currentDateTimeUtc();
    QDateTime nowKST = convertKST(nowUTC);

    if(dtKST.date() == nowKST.date())
        ui->dateDial->setValue(1);
    else
        ui->dateDial->setValue(0);
    ui->hourDial->setValue(dtKST.time().hour());
    ui->minDial->setValue(dtKST.time().minute());
    ui->secDial->setValue(dtKST.time().second());
}

QDateTime MainWindow::findDataTimeKSTFromDial()
{
    QDateTime dtKST, nowKST, nowUTC;
    nowUTC = QDateTime::currentDateTimeUtc();
    nowKST = convertKST(nowUTC);
    dtKST.setTimeSpec(Qt::UTC);

    if(ui->dateDial->value() == 1)
        dtKST.setDate(nowKST.date());
    else
        dtKST.setDate(nowKST.addDays(-1).date());

    int hour = ui->hourDial->value();
    int min = ui->minDial->value();
    int sec = ui->secDial->value();
    QTime t; t.setHMS(hour, min, sec);
    dtKST.setTime(t);

    return dtKST;
}

void MainWindow::dDialChanged(int date)
{
    if(isRealTimeMode)
        return;

    if(isStopMode)
    {
        QDateTime nowUTC, nowKST;
        nowUTC = QDateTime::currentDateTimeUtc();
        nowKST = convertKST(nowUTC);

        if(date != 1)
            nowKST = nowKST.addDays(-1);

        ui->dateLB->setText(nowKST.toString("MM-dd"));
    }
}

void MainWindow::hDialChanged(int hour)
{
    if(isRealTimeMode)
        return;

    if(isStopMode)
        ui->hourLB->setText(QString::number(hour));
}

void MainWindow::mDialChanged(int min)
{
    if(isRealTimeMode)
        return;

    if(isStopMode)
        ui->minLB->setText(QString::number(min));
}

void MainWindow::sDialChanged(int sec)
{
    if(isRealTimeMode)
        return;

    if(isStopMode)
        ui->secLB->setText(QString::number(sec));
}

void MainWindow::eventListCBChanged(int eventIndex)
{
    if(eventIndex == 0 || eventIndex == -1)
        return;

    QDateTime etKST = QDateTime::fromString(ui->eventListCB->currentText().section(" M", 0, 0),
                                                "yyyy-MM-dd hh:mm:ss");

    QDateTime nowKST = QDateTime::currentDateTimeUtc();
    nowKST = convertKST(nowKST);

    if(etKST.date() == nowKST.date())
        ui->dateDial->setValue(1);
    else
        ui->dateDial->setValue(0);

    etKST = etKST.addSecs(-2);

    ui->hourDial->setValue(etKST.time().hour());
    ui->minDial->setValue(etKST.time().minute());
    ui->secDial->setValue(etKST.time().second());
}

void MainWindow::doRepeatWork()
{
    if(isRealTimeMode)
    {
        dataTimeKST = serverTimeUtc.addSecs(- SECNODS_FOR_ALIGN_QSCD); // GMT
        dataTimeKST = convertKST(dataTimeKST);

        QString stylesheet("background-color: rgb(52, 101, 164);");
        ui->realtimePB->setStyleSheet(stylesheet);
    }
    else
    {
        dataTimeKST = findDataTimeKSTFromDial();
        dataTimeKST = dataTimeKST.addSecs(1);

        QString stylesheet("background-color: rgb(52, 101, 164);");
        ui->playPB->setStyleSheet(stylesheet);
    }

    QString stylesheet2("background-color: rgb(238, 238, 236);");
    ui->stopPB->setStyleSheet(stylesheet2);

    setDialAndLCDUsingKST(dataTimeKST);


    bool valid = timeCheck(dataTimeKST);
    if(valid)
    {
        QDateTime dataTimeUTC = convertUTC(dataTimeKST);
        recvEEWMessage->sendTextMessage(QString::number(dataTimeUTC.toTime_t()));
        recvPOINTMessage->sendTextIncludeOptionMessage(QString::number(dataTimeUTC.toTime_t()));

    }
    else
    {
        _BINARY_POINT_PACKET pointpacket;
        _BINARY_PGA_PACKET stationpacket;
        pointpacket.dataTime = 0;
        stationpacket.dataTime = 0;
        stationpacket.numStation = 0;
        native->animate(eewpacket, pointpacket, stationpacket);
    }
}

bool MainWindow::timeCheck(QDateTime dt)
{
    QDateTime dtUTC = convertUTC(dt);
    QDateTime nowUTC = QDateTime::currentDateTimeUtc();

    if(dtUTC > nowUTC)
        return false;
    else
    {
        int diff = nowUTC.toTime_t() - dtUTC.toTime_t();
        if(diff > KEEP_LARGE_DATA_DURATION)
            return false;
        else
            return true;
    }
}

void MainWindow::rvEEWMessageFromThread(_BINARY_SMALL_EEWLIST_PACKET packet)
{
    eewpacket = packet;

    ui->eventListCB->clear();

    QStringList events;
    events << "Select a Available EEW Event";

    if(eewpacket.numEEW != 0)
    {
        for(int i=eewpacket.numEEW-1;i>=0;i--)
        {
            QDateTime etKST;
            etKST.setTimeSpec(Qt::UTC);
            etKST.setTime_t(eewpacket.eewInfos[i].origintime);
            etKST = convertKST(etKST);

            QString eventName = etKST.toString("yyyy-MM-dd hh:mm:ss") + " M" + QString::number(eewpacket.eewInfos[i].magnitude, 'f', 1);

            events << eventName;
        }
    }

    ui->eventListCB->addItems(events);
}

void MainWindow::rvPOINTSMessageFromThread(_BINARY_POINT_PACKET pointpacket)
{
    _BINARY_PGA_PACKET stationpacket;
    stationpacket.dataTime = 0;
    native->animate(eewpacket, pointpacket, stationpacket);
}

void MainWindow::rvSTATIONMessageFromThread(_BINARY_PGA_PACKET stationpacket)
{
    _BINARY_POINT_PACKET pointpacket;
    pointpacket.dataTime = 0;
    native->animate(eewpacket, pointpacket, stationpacket);
}

void MainWindow::rvTIMEMessageFromThread(int currentEpochTimeUTC)
{
    serverTimeUtc.setTime_t(currentEpochTimeUTC);
}
