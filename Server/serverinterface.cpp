#include "serverinterface.h"
#include "ui_serverinterface.h"

ServerInterface::ServerInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerInterface)   //создаем новый объект графического интерфейса
    , mainServer(new MainServer(this))    //создаем новый объект сервера
{
    ui->setupUi(this);
    connect(mainServer, SIGNAL(logMessage(QString)), this, SLOT(outputLogMessage(QString)));
    outputLogMessage("Запуск сервера...");
}

ServerInterface::~ServerInterface()
{
    delete ui;
}
void ServerInterface::outputLogMessage(QString message){
    ui->logBrowser->append(message);
}
