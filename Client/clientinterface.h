#ifndef CLIENTINTERFACE_H
#define CLIENTINTERFACE_H

#include <QWidget>
#include <QAbstractSocket>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHostAddress>
#include "client.h"
namespace Ui {
class ClientInterface;
}

class ClientInterface : public QWidget  //класс графического интерфейса клиента
{
    Q_OBJECT    //подключение мета-объектной системы
    Q_DISABLE_COPY(ClientInterface)  //отключение копирования
public:
    explicit ClientInterface(QWidget *parent = nullptr);
    ~ClientInterface();
private:
    Ui::ClientInterface *ui;    //объект графического интерфейса
    Client *chatClient;   //объект логики клиента
    QStandardItemModel *chatModel;    //модель для представления данных, в нашем случае - вывода сообщений
    QString name;
    QString nameSender;
private slots:
    void connectedToServer();   //слот подключения к серверу
    void messageReceived(QString text);   //слот получения сообщения
    void attemptConnection();   //слот вызова подключения к серверу
    void clearChat();
    void sendMessage(); //слот отправки сообщения
    void disconnectedFromServer(); //слот отключения от сервера
};

#endif // CLIENTINTERFACE_H
