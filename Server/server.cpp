#include "server.h"

Server::Server(QObject *parent)
    : QObject(parent)
    , serverSocket(new QTcpSocket(this))
{
    connect(serverSocket, &QTcpSocket::readyRead, this, &Server::receiveMessage); //установка соединения для чтения сообщений
    connect(serverSocket, &QTcpSocket::disconnected, this, &Server::disconnectFromClient);  //установка соединения для отключения клиента
}
bool Server::setSocketDescriptor(qintptr socketDescriptor)
{
    return serverSocket->setSocketDescriptor(socketDescriptor);
}
void Server::disconnectFromClient()    //слот отключения клиента
{
    emit sendEveryone("DISCONNECT:"+userName);
    emit disconnectedFromClient(this);
    serverSocket->disconnectFromHost();
}

QString Server::getUserName()
{
    return userName;
}
void Server::receiveMessage()  //слот получения сообщения
{
    QByteArray byteArray = serverSocket->readLine();
    QString data = QString::fromUtf8(byteArray);
    if (data.split(" ").at(0)=="LOGIN"){    //если получили логин
        int index = data.indexOf(" ");
        data.chop(1);
        if (!data.remove(0, index + 1).isEmpty()){   //и он не пустой - сохраняем логин
            userName = data;
            emit sendEveryone("CONNECT:"+userName);
            emit logMessage("Подключение нового пользователя: " + userName);
        }
    }
    else if (data.split(":").at(0)=="GET MESSAGES"){
        qDebug()<<data;
        data.remove(0, data.indexOf(":")+1);
        emit getMessage(data.left(data.indexOf(":")), data.remove(0, data.indexOf(":")+1));
    }
    else{
        emit searchClient(userName, data);
    }
}
void Server::sendToClient(QString str){ //метод отправки клиентам сообщения
    serverSocket->write(str.toUtf8());
}
