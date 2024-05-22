#include "mainserver.h"

MainServer::MainServer(QObject *parent)
    : QTcpServer(parent)
{
    //задаем ip-адрес и порт
    QHostAddress ipAddress("127.0.0.1");
    quint16 port = 2323;
    if(this->listen(ipAddress, port)) {
        qDebug() << "Сервер начал работу, IP-адрес:" << ipAddress.toString() << " порт:" << port;
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

