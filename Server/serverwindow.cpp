#include "serverwindow.h"
#include "ui_serverwindow.h"

ServerWindow::ServerWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ServerWindow)
    , mainServer(new MainServer(this))
{
    ui->setupUi(this);
    //установка соединения сигнала и слота вывода логов в окне сервера
    connect(mainServer, &MainServer::logMessage, this, &ServerWindow::outputLogMessage);
    outputLogMessage("Запуск сервера...");
}

ServerWindow::~ServerWindow()
{
    delete ui;
}
void ServerWindow::outputLogMessage(QString message){   //вывод в окне логов
    ui->logBrowser->append(message);
}
