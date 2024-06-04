#ifndef SERVERCONNECTION_H
#define SERVERCONNECTION_H

#include <QObject>
#include <QTcpSocket>
#include <QDataStream>
#include <QJsonParseError>
#include <QJsonObject>
class ServerConnection : public QObject   //класс подключения сервера и клиента
{
    Q_OBJECT    //подключение системы сигналов-слотов
public:
    explicit ServerConnection(QObject *parent = nullptr);
    virtual bool setSocketDescriptor(qintptr socketDescriptor); //установка значения сокета для соединения с клиентом
    QString getUserName();
    void sendToClient(QString sender, QString text, QString time); //отправка сообщения клиенту
    void sendStatusToClient(QString status, QString user);    //отправка данных об изменении статуса клиента
public slots:
    void disconnectFromClient();    //слот отключения клиента
signals:
    void sendEveryone(QString status, QString user); //сигнал отправки сообщения о статусе пользователя всем пользователям
    void searchClient(QString sender, QString recipient, QString text, QString time); //сигнал поиска клиента для отправки сообщения
    void disconnectedFromClient(ServerConnection* server);  //сигнал отключения клиента
    void logMessage(QString log);   //сигнал для отправки лога
    void getMessages(QString sender, QString recipient); //сигнал получения списка сообщений двух пользователей
private slots:
    void read();  //слот для чтения данных с сокета
private:
    QTcpSocket *serverSocket; //сокет сервера
    QString userName; //логин пользователя
};

#endif // SERVERCONNECTION_H
