#include <QApplication>
#include "clientwindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ClientWindow interface;
    interface.setWindowTitle("Чат");
    interface.show();
    return a.exec();
}
