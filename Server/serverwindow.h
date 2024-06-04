#ifndef SERVERWINDOW_H
#define SERVERWINDOW_H

#include <QWidget>
#include "mainserver.h"
namespace Ui {
class ServerWindow;
}

class ServerWindow : public QWidget  //класс графического интерфейса сервера
{
    Q_OBJECT    //подключение системы сигналов-слотов

public:
    explicit ServerWindow(QWidget *parent = nullptr);
    ~ServerWindow();

private:
    Ui::ServerWindow *ui;    //объект графического интерфейса
    MainServer *mainServer;   //основной объект сервера
private slots:
    void outputLogMessage(QString message); //слот для вывода логов
};

#endif // SERVERWINDOW_H
