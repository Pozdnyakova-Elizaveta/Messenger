/********************************************************************************
** Form generated from reading UI file 'serverinterface.ui'
**
** Created by: Qt User Interface Compiler version 5.15.10
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SERVERINTERFACE_H
#define UI_SERVERINTERFACE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ServerInterface
{
public:

    void setupUi(QWidget *ServerInterface)
    {
        if (ServerInterface->objectName().isEmpty())
            ServerInterface->setObjectName(QString::fromUtf8("ServerInterface"));
        ServerInterface->resize(400, 300);

        retranslateUi(ServerInterface);

        QMetaObject::connectSlotsByName(ServerInterface);
    } // setupUi

    void retranslateUi(QWidget *ServerInterface)
    {
        ServerInterface->setWindowTitle(QCoreApplication::translate("ServerInterface", "Form", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ServerInterface: public Ui_ServerInterface {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SERVERINTERFACE_H
