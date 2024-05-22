#include "serverinterface.h"
#include "ui_serverinterface.h"

ServerInterface::ServerInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerInterface)   //создаем новый объект графического интерфейса
    , mainServer(new MainServer(this))    //создаем новый объект сервера
{
    ui->setupUi(this);
}

ServerInterface::~ServerInterface()
{
    delete ui;
}
