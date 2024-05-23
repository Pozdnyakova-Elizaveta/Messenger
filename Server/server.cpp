#include "server.h"

Server::Server(QObject *parent)
    : QObject(parent)
    , serverSocket(new QTcpSocket(this))
{
    connect(serverSocket, &QTcpSocket::readyRead, this, &Server::receiveMessage); //установка соединения для чтения сообщений
    connect(serverSocket, &QTcpSocket::disconnected, this, &Server::disconnectedFromClient);  //установка соединения для отключения клиента
}
bool Server::setSocketDescriptor(qintptr socketDescriptor)
{
    return serverSocket->setSocketDescriptor(socketDescriptor);
}
void Server::disconnectFromClient()    //слот отключения клиента
{
    emit sendEveryone("DISCONNECT:"+userName);
    emit disconnectedFromClient();
    qDebug()<<"Отключение";
    serverSocket->disconnectFromHost();
}

QString Server::getUserName()
{
    return userName;
}
void Server::receiveMessage()  //слот получения сообщения
{
    QByteArray byteArray = serverSocket->readAll();
    QString data = QString::fromUtf8(byteArray);
    if (data.split(" ").at(0)=="LOGIN"){    //если получили логин
        int index = data.indexOf(" ");
        if (!data.remove(0, index + 1).isEmpty()){   //и он не пустой - сохраняем логин
            userName = data;
            emit sendEveryone("CONNECT:"+userName);
        }
    }
    else{
        emit searchClient(userName, data);
    }
}
void Server::sendToClient(QString str){ //метод отправки клиентам сообщения
    serverSocket->write(str.toUtf8());
}
