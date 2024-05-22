#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QTime>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

class Client : public QObject   //класс логики клиента
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(Client)  //отключение копирования
public:
    explicit Client(QObject *parent = nullptr);
public slots:
    void connectToServer();     //слот для подключения к серверу
    void sendLogin(QString userName);    //слот для отправки логина
    void sendMessage(QString text);  //слот для отправки сообщения
    void disconnectFromServer();  //слот для отключения от сервера
private slots:
    void onReadyRead(); //слот для реакции на наличие данных в сокете
signals:
    void connected();   //сигнал подключения к серверу
    void disconnected();    //сигнал отключения от сервера
    void messageReceived(QString sender, QString text);   //сигнал получения сообщения
private:
    QTcpSocket *clientSocket; //сокет клиента
};

#endif // CLIENT_H
