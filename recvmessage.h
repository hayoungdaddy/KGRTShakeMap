#ifndef RECVMESSAGE_H
#define RECVMESSAGE_H

#include "common.h"

#include <QThread>
#include <QtWebSockets/QtWebSockets>

class RecvMessage : public QThread
{
    Q_OBJECT
public:
    RecvMessage(QWidget *parent = nullptr);
    ~RecvMessage();

public:
    void setup(QUrl);
    void cleanUp();
    void sendTextMessage(QString);
    void sendTextIncludeOptionMessage(QString);
    int chanID;
    int dataType;
    void setChanID(int);
    void setDataType(int);

private:
    QWebSocket m_webSocket;
    QUrl m_url;

private slots:
    void onConnected();
    void onTextMessageReceived(QString);
    void onBinaryMessageReceived(QByteArray);

signals:
    void sendPOINTSMessageToMainWindow(_BINARY_POINT_PACKET);
    void sendEEWMessageToMainWindow(_BINARY_EEW_PACKET);
    void sendSTATIONMessageToMainWindow(_BINARY_STATION_PACKET);
};

#endif // RECVMESSAGE_H
