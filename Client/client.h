#ifndef CLIENT_H
#define CLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDataStream>

class Client : public QObject   //класс логики клиента
{
    Q_OBJECT    //подключение системы сигналов-слотов
public:
    explicit Client(QObject *parent = nullptr);
    void sendLogin(QString userName);    //метод для отправки логина на сервер
    void sendMessage(QString sender, QString recipient, QString text, QString time);  //метод для отправки сообщения на сервер
    void sendMessageRequest(QString sender, QString recipient); //метод для отправки запроса на получение сообщений с базы данных
    void sendDatagram();    //метод отправки датаграммы для определения адреса сервера
signals:
    void connected();   //сигнал подключения к серверу
    void disconnected();    //сигнал отключения от сервера
    void messageReceived(QString sender, QString text, QString time);   //сигнал получения сообщения с сервера
    void statusReceived(QString status, QString user);  //сигнал получения данных об изменении состояния другого пользователя
private slots:
    void read(); //слот для чтения данных с сокета
    void readDatagram();    //слот для чтения полученной датаграммы
private:
    QTcpSocket *clientSocket; //TCP-сокет клиента
    QUdpSocket *udpClientSocket; //UDP-сокет клиента
    void connectToServer(QString IPServer, QString IPClient, quint16 port);     //метод для подключения к серверу по полученному IP-адресу
};

#endif // CLIENT_H
