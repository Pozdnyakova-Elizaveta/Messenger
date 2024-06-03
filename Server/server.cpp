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
    emit sendEveryone("DISCONNECT", userName);
    emit disconnectedFromClient(this);
    serverSocket->disconnectFromHost();
}

QString Server::getUserName()
{
    return userName;
}
void Server::receiveMessage()  //слот получения сообщения
{
    QByteArray jsonData;
    QDataStream socketStream(serverSocket);
    while (!socketStream.atEnd()) {
        socketStream >> jsonData;
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData, &parseError);
            if (parseError.error == QJsonParseError::NoError) {
                if (jsonDoc.isObject()){
                    QJsonObject jsonObject = jsonDoc.object();
                    QJsonValue type = jsonObject.value("Type");
                    if (type.toString().compare("Login")==0){
                        userName = jsonObject.value("Login").toString();
                        emit sendEveryone("CONNECT", userName);
                        emit logMessage("Подключение нового пользователя: " + userName);
                    }
                    if (type.toString().compare("Get Messages")==0){
                        QString sender = jsonObject.value("Sender").toString();
                        QString recipient = jsonObject.value("Recipient").toString();
                        emit getMessage(sender, recipient);
                    }
                    if (type.toString().compare("Message")==0){
                        QString sender = jsonObject.value("Sender").toString();
                        QString recipient = jsonObject.value("Recipient").toString();
                        QString messageText = jsonObject.value("Message text").toString();
                        QString time = jsonObject.value("Time").toString();
                        emit searchClient(sender, recipient, messageText, time);
                    }
            }
            }
    }
}
void Server::sendToClient(QString sender, QString text, QString time){ //метод отправки клиентам сообщения
    QJsonObject message;
    message.insert("Type", QJsonValue::fromVariant("Message"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Message text", QJsonValue::fromVariant(text));
    message.insert("Time", QJsonValue::fromVariant(time));
    QDataStream serverStream(serverSocket);
    serverStream << QJsonDocument(message).toJson();
}
void Server::sendStatusClient(QString status, QString user){
    qDebug()<<status;
    QJsonObject message;
    message.insert("Type", QJsonValue::fromVariant("Status"));
    message.insert("Status", QJsonValue::fromVariant(status));
    message.insert("Sender", QJsonValue::fromVariant(user));
    QDataStream serverStream(serverSocket);
    serverStream << QJsonDocument(message).toJson();
}
