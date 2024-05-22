#ifndef SERVERINTERFACE_H
#define SERVERINTERFACE_H

#include <QWidget>
#include <QMessageBox>
#include "mainserver.h"
namespace Ui {
class ServerInterface;
}

class ServerInterface : public QWidget  //класс графического интерфейса сервера
{
    Q_OBJECT    //подключение мета-объектной системы

public:
    explicit ServerInterface(QWidget *parent = nullptr);
    ~ServerInterface();

private:
    Ui::ServerInterface *ui;    //объект графического интерфейса
    MainServer *mainServer;   //основной объект сервера
};

#endif // SERVERINTERFACE_H
