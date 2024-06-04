#ifndef CLIENTWINDOW_H
#define CLIENTWINDOW_H

#include <QWidget>
#include <QAbstractSocket>
#include <QMenu>
#include <QStandardItemModel>
#include <QInputDialog>
#include <QMessageBox>
#include <QHostAddress>
#include "client.h"
namespace Ui {
class ClientWindow;
}

class ClientWindow : public QWidget  //класс графического интерфейса клиента
{
    Q_OBJECT    //подключение системы сигналов-слотов
public:
    explicit ClientWindow(QWidget *parent = nullptr);
    ~ClientWindow();
private:
    Ui::ClientWindow *ui;    //объект графического интерфейса
    Client *chatClient;   //объект логики клиента
    QStandardItemModel *chatModel;    //модель для вывода сообщений в окне
    QString name;   //имя пользователя
    QString forwardMessageText; //пересылаемое сообщение
    QMenu forwardingMenu; //меню для выбора пользователя для пересылки сообщения
    void outputMessage(QString message, Qt::AlignmentFlag flag);    //вывод сообщения в окне
private slots:
    void connectedToServer();   //слот подключения к серверу
    void messageReceived(QString sender, QString text, QString time);   //слот для получения сообщения пользователя
    void statusReceived(QString status, QString user);  //слот для получения данных об изменении статуса пользователя
    void searchServer();   //слот вызова определения IP сервера
    void updateChat();  //слот обновления чата при выборе другого пользователя для переписки
    void sendMessage(); //слот отправки сообщения пользователю
    void disconnectedFromServer(); //слот отключения от сервера
    void openForwardMenu(QModelIndex index);    //слот открытия меню пересылки сообщения
    void forwardMessage(QAction *action);    //слот для пересылки сообщения выбранному пользователю
};

#endif // CLIENTWINDOW_H
