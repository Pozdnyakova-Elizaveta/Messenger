#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include "server.h"
class MainServer: public QTcpServer //класс сервера чата
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(MainServer)  //отключение копирования
public:
    explicit MainServer(QObject *parent = nullptr);
    void incomingConnection(qintptr socketDescriptor) override; //метод попытки подключения клиента к серверу
public slots:
    void stopServer();  //слот отключения сервера
private slots:
    void sendEveryone(QString message);    //слот отправки сообщений всем пользователям
private:
    QVector<Server *> clients;    //список подключенных клиентов
};

#endif // CHATSERVER_H
