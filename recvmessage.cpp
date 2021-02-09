#include "recvmessage.h"

RecvMessage::RecvMessage(QWidget *parent)
{
    connect(&m_webSocket, &QWebSocket::connected, this, &RecvMessage::onConnected);
    connect(&m_webSocket, &QWebSocket::disconnected, this, &RecvMessage::cleanUp);
}

RecvMessage::~RecvMessage()
{
}

void RecvMessage::setup(QUrl server)
{
    m_webSocket.open(server);
}

void RecvMessage::onConnected()
{
    connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &RecvMessage::onTextMessageReceived);
    connect(&m_webSocket, &QWebSocket::binaryMessageReceived, this, &RecvMessage::onBinaryMessageReceived);

    m_webSocket.sendTextMessage(QStringLiteral("Hello"));
}

void RecvMessage::onBinaryMessageReceived(QByteArray data)
{
    if(data.length() > 500000) // _BINARY_POINT_PACKET
    {
        _BINARY_POINT_PACKET myp;
        memcpy(&myp, data.data(), sizeof(_BINARY_POINT_PACKET));

        emit(sendPOINTSMessageToMainWindow(myp));
    }
    else if (data.length() < 1000) // _BINARY_EEW_PACKET
    {
        _BINARY_EEW_PACKET myp;
        memcpy(&myp, data.data(), sizeof(_BINARY_EEW_PACKET));

        emit(sendEEWMessageToMainWindow(myp));
    }
    else if (data.length() > 10000 && data.length() < 15000)
    {
        _BINARY_STATION_PACKET myp;
        memcpy(&myp, data.data(), sizeof(_BINARY_STATION_PACKET));

        emit(sendSTATIONMessageToMainWindow(myp));
    }
}

void RecvMessage::onTextMessageReceived(QString message)
{
    //m_webSocket.close();
}

void RecvMessage::cleanUp()
{
    m_webSocket.close();
    this->destroyed();
    this->deleteLater();
}

void RecvMessage::sendTextMessage(QString dataTime)
{
    m_webSocket.sendTextMessage(dataTime);
}

void RecvMessage::sendTextIncludeOptionMessage(QString dataTime)
{
    QString message;
    message = QString::number(chanID) + "_" + dataTime + "_" + QString::number(dataType);
    m_webSocket.sendTextMessage(message);
}

void RecvMessage::setChanID(int id)
{
    chanID = id;
}

void RecvMessage::setDataType(int type)
{
    dataType = type;
}
