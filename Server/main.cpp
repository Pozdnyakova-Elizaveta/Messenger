#include <QApplication>
#include "serverinterface.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ServerInterface server;
    server.show();
    return a.exec();
}
