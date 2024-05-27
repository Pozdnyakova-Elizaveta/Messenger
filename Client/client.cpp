#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent)
    , clientSocket(new QTcpSocket(this))  //создание сокета клиента
    , udpClientSocket(new QUdpSocket(this))
{
    connect(clientSocket, &QTcpSocket::connected, this, &Client::connected);  //установка соединения сигнала и слота для подключения к серверу
    connect(clientSocket, &QTcpSocket::disconnected, this, &Client::disconnected);  //установка соединения сигнала и слота для отключения от сервера
    connect(clientSocket, &QTcpSocket::readyRead, this, &Client::onReadyRead);    //установка соединения сигнала и слота для получения данных с сокета
    connect(udpClientSocket, &QUdpSocket::readyRead, this, &Client::connectToServer);
}
void Client::sendLogin(QString userName) //слот для отправки логина
{
    QString data;
    data.append("LOGIN ").append(userName); //добавление информации, что отправляется логин
    clientSocket->write(data.toUtf8()); //отправка логина на сервер
}

void Client::sendMessage(QString text)   //слот для отправки сообщения
{
    clientSocket->write(text.toUtf8());
}
void Client::disconnectFromServer()  //слот для отключения от сервера
{
    clientSocket->disconnectFromHost();
}
void Client::connectToServer()     //слот для подключения к серверу
{
    while (udpClientSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpClientSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpClientSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        QRegExp reg("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");
        if (reg.indexIn(datagram)!=-1){
            if(datagram==sender.toString()) clientSocket->connectToHost("127.0.0.1", senderPort);
            else clientSocket->connectToHost(sender, senderPort);
        }
    }
}
void Client::sendDatagram(){
    QByteArray datagram = "BroadcastRequest";
    udpClientSocket->writeDatagram(datagram, QHostAddress::Broadcast, 2323);
}
void Client::onReadyRead() //слот для реакции на наличие данных в сокете
{
    QTextStream in(clientSocket);
    in.setCodec("UTF-8");
    while (!in.atEnd()) {
        QByteArray byteArray = clientSocket->readLine();
        QString data = QString::fromUtf8(byteArray);
        if ((data.startsWith("CONNECT") || data.startsWith("DISCONNECT")) && data.endsWith("\n")) data.chop(1);
        messageReceived(data);
    }
}
