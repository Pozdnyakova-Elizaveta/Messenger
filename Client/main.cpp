#include <QApplication>
#include "clientinterface.h".h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientInterface interface;
    interface.show();
    return a.exec();
}
