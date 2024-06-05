#include <QApplication>
#include "serverwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerWindow server;
    server.setWindowTitle("Сервер");
    server.show();
    return a.exec();
}
