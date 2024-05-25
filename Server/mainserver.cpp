#include "mainserver.h"

MainServer::MainServer(QObject *parent)
    : QTcpServer(parent)
{
    //задаем ip-адрес и порт
    quint16 port = 2323;
    this->listen(QHostAddress::LocalHost, port);
    log.setFileName("log.txt");
    log.open(QIODevice::Append | QIODevice::Text);
    if (log.isOpen())
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "\nЗапуск сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
}

MainServer::~MainServer()
{
    for (Server *worker : clients) {
        worker->disconnectFromClient();
    }
    if (log.isOpen())
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "Отключение сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
    log.close();
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
    connect(server, SIGNAL(logMessage(QString)), this, SLOT(sendLogMessage(QString)));
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

void MainServer::searchClient(QString sender, QString message){
    int index = message.indexOf(":");
    QString login = message.split(":").at(0);
    qDebug()<<"Ищем:"<<login;
    for (Server *worker : clients) {
        if (worker->getUserName()==login){
            worker->sendToClient(sender+message.remove(0, index));
            emit logMessage("Отправка сообщения, отправитель: "+sender+", получатель: "+worker->getUserName());
            if (log.isOpen())
            {
                QByteArray data = QString("Отправка сообщения, отправитель: "+sender+", получатель: "+worker->getUserName()+"\n").toUtf8();
                log.write(data);
            }
        }
    }
}
void MainServer::disconnectClient(Server* sender){
    emit logMessage("Отключение от сервера пользователя: "+sender->getUserName());
    if (log.isOpen())
    {
        QByteArray data = QString("Отключение от сервера пользователя: "+sender->getUserName()+"\n").toUtf8();
        log.write(data);
    }
    clients.removeAll(sender);
}
void MainServer::sendLogMessage(QString message){
    emit logMessage(message);
    if (log.isOpen())
    {
        QByteArray data = QString(message+"\n").toUtf8();
        log.write(data);
    }
}
