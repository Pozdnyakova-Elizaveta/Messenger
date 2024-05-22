#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent)
    , clientSocket(new QTcpSocket(this))  //создание сокета клиента
{
    connect(clientSocket, &QTcpSocket::connected, this, &Client::connected);  //установка соединения сигнала и слота для подключения к серверу
    connect(clientSocket, &QTcpSocket::disconnected, this, &Client::disconnected);  //установка соединения сигнала и слота для отключения от сервера
    connect(clientSocket, &QTcpSocket::readyRead, this, &Client::onReadyRead);    //установка соединения сигнала и слота для получения данных с сокета
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
    clientSocket->connectToHost("127.0.0.1", 2323);
}
void Client::onReadyRead() //слот для реакции на наличие данных в сокете
{
    //считывание данных с сокета
    QByteArray byteArray = clientSocket->readAll();
    QString data = QString::fromUtf8(byteArray);
    int index = data.indexOf(":");  //индекс разделения логина отправителя и сообщения
    emit messageReceived(data.split(":").at(0), data.remove(0, index + 1));
}
