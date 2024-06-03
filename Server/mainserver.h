#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include <QUdpSocket>
#include <QDateTime>
#include "server.h"
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

class MainServer: public QTcpServer //класс сервера чата
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(MainServer)  //отключение копирования
public:
    explicit MainServer(QObject *parent = nullptr);
    ~MainServer();
    void incomingConnection(qintptr socketDescriptor) override; //метод попытки подключения клиента к серверу
    void addMessage(QString sender, QString recipient, QString message, QString time);
private slots:
    void sendEveryone(QString status, QString message);    //слот отправки сообщений всем пользователям
    void searchClient(QString sender, QString recipient, QString message, QString time);
    void disconnectClient(Server* sender);
    void sendLogMessage(QString message);
    void udpAnswer();
    void getMessagesSlot(QString sender, QString recipient);
signals:
    void logMessage(QString message);
private:
    QVector<Server *> clients;    //список подключенных клиентов
    QUdpSocket udpServerSocket; //сокет сервера
    QFile log;
    QSqlDatabase db;
};

#endif // CHATSERVER_H
