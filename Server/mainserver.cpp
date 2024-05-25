#include "mainserver.h"

MainServer::MainServer(QObject *parent)
    : QTcpServer(parent)
{
    //задаем ip-адрес и порт
    quint16 port = 2323;
    if(this->listen(QHostAddress::Any, port)) {
        qDebug() << "Сервер начал работу";
    } else {
        qDebug() << "Ошибка запуска сервера";
    }

}

void MainServer::incomingConnection(qintptr socketDescriptor)   //метод попытки подключения клиента к серверу
{
    Server *server = new Server(this);
    if (!server->setSocketDescriptor(socketDescriptor)) {
        server->deleteLater();
        return;
    }
    //установка соединений сигналов объекта сервера для подключения клиентов со слотами основного сервера чата
    connect(server, SIGNAL(sendEveryone(QString)), this, SLOT(sendEveryone(QString)));
    connect(server, SIGNAL(searchClient(QString, QString)), this, SLOT(searchClient(QString, QString)));
    connect(server, &Server::disconnectedFromClient, this, std::bind(&MainServer::disconnectClient, this, server));
    for (Server *worker : clients) {
        server->sendToClient("CONNECT:"+worker->getUserName()+"\n");
    }
    clients.append(server);
}

void MainServer::sendEveryone(QString message)    //слот отправки сообщений всем пользователям
{
    for (Server *worker : clients) {
        Q_ASSERT(worker);
        if (worker == sender())
            continue;
        worker->sendToClient(message);
    }
}

void MainServer::stopServer()  //слот отключения сервера
{
    for (Server *worker : clients) {
        worker->disconnectFromClient();
    }
    close();
}
void MainServer::searchClient(QString sender, QString message){
    int index = message.indexOf(":");
    QString login = message.split(":").at(0);
    qDebug()<<"Ищем:"<<login;
    for (Server *worker : clients) {
        if (worker->getUserName()==login)
            worker->sendToClient(sender+message.remove(0, index));
    }
}
void MainServer::disconnectClient(Server* sender){
    clients.removeAll(sender);
}

