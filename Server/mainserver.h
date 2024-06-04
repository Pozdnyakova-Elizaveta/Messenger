#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QTcpServer>
#include <QUdpSocket>
#include <QDateTime>
#include "serverconnection.h"
#include <QFile>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>

class MainServer: public QTcpServer //класс главного сервера чата, объединяющего все подключения
{
    Q_OBJECT    //подключение системы сигналов-слотов

public:
    explicit MainServer(QObject *parent = nullptr);
    ~MainServer();
    void incomingConnection(qintptr socketDescriptor) override; //метод подключения клиента к серверу

signals:
    void logMessage(QString message);   //сигнал отправки лога

private:
    void addMessage(QString sender, QString recipient, QString message, QString time);  //метод добавления нового сообщения в базу данных
    QVector<ServerConnection *> clients;    //список подключенных клиентов
    QUdpSocket udpServerSocket;   //udp-сокет сервера
    QFile log;  //файл для записи логов
    QSqlDatabase db;    //объект подключения к базе данных

private slots:
    void sendEveryone(QString status, QString userName);    //слот отправки особщения о статусе пользователя всем пользователям
    void searchClient(QString sender, QString recipient, QString message, QString time);     //слот поиска пользователя для отправки сообщения
    void disconnectClient(ServerConnection* sender);  //слот отключения клиента
    void sendLogMessage(QString message);   //слот отправки сообщения лога графическому интерфейсу
    void udpAnswer();   //слот отправки ответной датаграммы клиенту
    void getMessages(QString sender, QString recipient);    //слот получения сообщений диалога из базы данных
};

#endif // CHATSERVER_H
