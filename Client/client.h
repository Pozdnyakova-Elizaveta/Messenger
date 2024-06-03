#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QDataStream>
#include <QTime>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

class Client : public QObject   //класс логики клиента
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(Client)  //отключение копирования
public:
    explicit Client(QObject *parent = nullptr);
public slots:
    void connectToServer();     //слот для подключения к серверу
    void sendLogin(QString userName);    //слот для отправки логина
    void sendMessage(QString sender, QString recipient, QString text);  //слот для отправки сообщения
    void sendMessageRequest(QString sender, QString recipient);
    void disconnectFromServer();  //слот для отключения от сервера
    void sendDatagram();
private slots:
    void onReadyRead(); //слот для реакции на наличие данных в сокете
signals:
    void connected();   //сигнал подключения к серверу
    void disconnected();    //сигнал отключения от сервера
    void messageReceived(QString sender, QString text, QString time);   //сигнал получения сообщения
    void statusReceived(QString status, QString user);
private:
    QTcpSocket *clientSocket; //сокет клиента
    QUdpSocket *udpClientSocket; //сокет клиента
};

#endif // CLIENT_H
