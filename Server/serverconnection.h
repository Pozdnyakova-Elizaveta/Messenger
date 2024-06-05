#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslPreSharedKeyAuthenticator>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonObject>
#include <QFile>
class ServerConnection : public QObject   //класс подключения сервера и клиента
{
    Q_OBJECT    //подключение системы сигналов-слотов

public:
    explicit ServerConnection(QObject *parent = nullptr);
    bool setSocketDescriptor(qintptr socketDescriptor); //установка значения сокета для соединения с клиентом
    QString getUserName();
    void sendToClient(QString sender, QString text, QString time); //отправка сообщения клиенту
    void sendStatusToClient(QString status, QString user);    //отправка данных об изменении статуса клиента
    void start();

public slots:
    void disconnectFromClient();    //слот отключения клиента

signals:
    void sendEveryone(QString status, QString user); //сигнал отправки сообщения о статусе пользователя всем пользователям
    void searchClient(QString sender, QString recipient, QString text, QString time); //сигнал поиска клиента для отправки сообщения
    void disconnectedFromClient(ServerConnection* server);  //сигнал отключения клиента
    void logMessage(QString log);   //сигнал для отправки лога
    void getMessages(QString sender, QString recipient); //сигнал получения списка сообщений двух пользователей

private:
    QSslSocket *serverSocket; //сокет сервера
    QString username; //логин пользователя

private slots:
    void read();  //слот для чтения данных с сокета
    void handlingSslError(const QList<QSslError>& errors);   //слот для обработки ошибок ssl
};

#endif // SERVERCONNECTION_H
