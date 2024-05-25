#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include <QDateTime>
#include "server.h"
#include <QFile>
class MainServer: public QTcpServer //класс сервера чата
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(MainServer)  //отключение копирования
public:
    explicit MainServer(QObject *parent = nullptr);
    ~MainServer();
    void incomingConnection(qintptr socketDescriptor) override; //метод попытки подключения клиента к серверу
private slots:
    void sendEveryone(QString message);    //слот отправки сообщений всем пользователям
    void searchClient(QString sender, QString message);
    void disconnectClient(Server* sender);
    void sendLogMessage(QString message);
signals:
    void logMessage(QString message);
private:
    QVector<Server *> clients;    //список подключенных клиентов
    QFile log;
};

#endif // CHATSERVER_H
