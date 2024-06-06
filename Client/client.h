#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QSslKey>
#include <QSslPreSharedKeyAuthenticator>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRegularExpression>
#include <QDataStream>
#include <QFile>

class Client : public QObject   //класс логики клиента
{
    Q_OBJECT    //подключение системы сигналов-слотов

public:
    explicit Client(QObject *parent = nullptr);
    void sendLogin(QString userName);    //метод для отправки логина на сервер
    void sendMessage(QString sender, QString recipient, QString text, QString time);  //метод для отправки сообщения на сервер
    void sendMessageRequest(QString sender, QString recipient); //метод для отправки запроса на получение сообщений с базы данных
    void connectToServer();     //метод для подключения к серверу по полученному IP-адресу
signals:
    void connected();   //сигнал подключения к серверу
    void disconnected();    //сигнал отключения от сервера
    void messageReceived(QString sender, QString text, QString time);   //сигнал получения сообщения с сервера
    void statusReceived(QString status, QString user);  //сигнал получения данных об изменении состояния другого пользователя

private:
    QSslSocket *clientSocket; //SSL-сокет клиента
    const QString IP_SERVER = "192.168.56.1";   //IP-адрес сервера

private slots:
    void read(); //слот для чтения данных с сокета
    void handlingSslError(const QList<QSslError>& errors);   //слот для обработки ошибок ssl
};

#endif // CLIENT_H
