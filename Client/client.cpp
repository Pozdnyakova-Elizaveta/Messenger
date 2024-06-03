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
void Client::sendLogin(QString userName){
    QJsonObject message;
    message.insert("Type", QJsonValue::fromVariant("Login"));
    message.insert("Login", QJsonValue::fromVariant(userName));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();
}
void Client::sendMessageRequest(QString sender, QString recipient){
    QJsonObject message;
    message.insert("Type", QJsonValue::fromVariant("Get Messages"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Recipient", QJsonValue::fromVariant(recipient));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();
}
void Client::sendMessage(QString sender, QString recipient, QString text)   //слот для отправки сообщения
{
    QJsonObject message;
    message.insert("Type", QJsonValue::fromVariant("Message"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Recipient", QJsonValue::fromVariant(recipient));
    message.insert("Message text", QJsonValue::fromVariant(text));
    message.insert("Time", QJsonValue::fromVariant(QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm")));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();
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
            else clientSocket->connectToHost(sender.toString(), senderPort);
        }
    }
}
void Client::sendDatagram(){
    QByteArray datagram = "BroadcastRequest";
    udpClientSocket->writeDatagram(datagram, QHostAddress::Broadcast, 2323);
}
void Client::onReadyRead() //слот для реакции на наличие данных в сокете
{
    QByteArray jsonData;
    QDataStream socketStream(clientSocket);
    while (!socketStream.atEnd()) {
        socketStream >> jsonData;
        QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()){
                    QJsonObject jsonObject = jsonDoc.object();
                    QJsonValue type = jsonObject.value("Type");
                    if (type.toString().compare("Status")==0){
                        QString sender = jsonObject.value("Sender").toString();
                        QString status = jsonObject.value("Status").toString();
                        emit statusReceived(status, sender);
                    }
                    if (type.toString().compare("Message")==0){
                        QString sender = jsonObject.value("Sender").toString();
                        QString text = jsonObject.value("Message text").toString();
                        QString time = jsonObject.value("Time").toString();
                        emit messageReceived(sender, text, time);
                    }
                }
            }
    }
}
