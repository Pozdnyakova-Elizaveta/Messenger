#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>


class Server : public QObject   //класс сервера для взаимодействия с клиентом
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(Server)  //отключение копирования
public:
    explicit Server(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor); //установка значения сокета для соединения с клиентом
    QString getUserName();
    void sendToClient(QString str); //отправка сообщения клиенту
signals:
    void sendEveryone(QString str); //сигнал отправки сообщения
    void messageReceived(QString str);  //сигнал получения сообщения
    void disconnectedFromClient();  //сигнал отключения клиента
public slots:
    void disconnectFromClient();    //слот отключения клиента
private slots:
    void receiveMessage();  //слот получения сообщения
private:
    QTcpSocket *serverSocket; //сокет сервера
    QString userName; //логин пользователя
};

#endif // SERVER_H
