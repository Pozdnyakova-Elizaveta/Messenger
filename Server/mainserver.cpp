#include "mainserver.h"

MainServer::MainServer(QObject *parent)
    : QTcpServer(parent)
{
    //задаем ip-адрес и порт
    quint16 port = 2323;
    this->listen(QHostAddress::LocalHost, port);
    log.setFileName("log.txt");
    log.open(QIODevice::Append | QIODevice::Text);
    if (log.isOpen())
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "\nЗапуск сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
    udpServerSocket.bind(2323, QUdpSocket::ShareAddress);
    connect(&udpServerSocket, &QUdpSocket::readyRead, this, &MainServer::udpAnswer);
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("Messages");
    db.setHostName("192.168.56.1");
    db.setUserName("postgres");
    db.setPassword("Postgres");
    if (!db.open()) {
        qDebug() << "Ошибка открытия базы данных:" << db.lastError().text();
    }
}

MainServer::~MainServer()
{
    for (Server *worker : clients) {
        worker->disconnectFromClient();
    }
    if (log.isOpen())
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "Отключение сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
    log.close();
}
void MainServer::incomingConnection(qintptr socketDescriptor)   //метод попытки подключения клиента к серверу
{
    Server *server = new Server(this);
    if (!server->setSocketDescriptor(socketDescriptor)) {
        server->deleteLater();
        return;
    }
    //установка соединений сигналов объекта сервера для подключения клиентов со слотами основного сервера чата
    connect(server, SIGNAL(sendEveryone(QString)), this, SLOT(sendEveryone(QString)));
    connect(server, SIGNAL(searchClient(QString, QString)), this, SLOT(searchClient(QString, QString)));
    connect(server, &Server::disconnectedFromClient, this, std::bind(&MainServer::disconnectClient, this, server));
    connect(server, SIGNAL(logMessage(QString)), this, SLOT(sendLogMessage(QString)));
    connect(server, SIGNAL(getMessage(QString, QString)), this, SLOT(getMessagesSlot(QString, QString)));
    for (Server *worker : clients) {
        server->sendToClient("CONNECT:"+worker->getUserName()+"\n");
    }
    clients.append(server);
}

void MainServer::sendEveryone(QString message)    //слот отправки сообщений всем пользователям
{
    for (Server *worker : clients) {
        Q_ASSERT(worker);
        if (worker == sender())
            continue;
        worker->sendToClient(message);
    }
}

void MainServer::searchClient(QString sender, QString message){
    int index = message.indexOf(":");
    QString login = message.split(":").at(0);
    for (Server *worker : clients) {
        if (worker->getUserName()==login){
            addMessage(sender, message);
            worker->sendToClient(sender+message.remove(0, index));
            emit logMessage("Отправка сообщения, отправитель: "+sender+", получатель: "+worker->getUserName());
            if (log.isOpen())
            {
                QByteArray data = QString("Отправка сообщения, отправитель: "+sender+", получатель: "+worker->getUserName()+"\n").toUtf8();
                log.write(data);
            }
        }
    }
}
void MainServer::disconnectClient(Server* sender){
    emit logMessage("Отключение от сервера пользователя: "+sender->getUserName());
    if (log.isOpen())
    {
        QByteArray data = QString("Отключение от сервера пользователя: "+sender->getUserName()+"\n").toUtf8();
        log.write(data);
    }
    clients.removeAll(sender);
}
void MainServer::sendLogMessage(QString message){
    emit logMessage(message);
    if (log.isOpen())
    {
        QByteArray data = QString(message+"\n").toUtf8();
        log.write(data);
    }
}
void MainServer::udpAnswer() {
    while (udpServerSocket.hasPendingDatagrams())
    {
        QByteArray datagram;
        datagram.resize(udpServerSocket.pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;
        udpServerSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);
        if (datagram=="BroadcastRequest"){
            QByteArray senderAddress = sender.toString().toUtf8();
            udpServerSocket.writeDatagram(senderAddress, sender, senderPort);
        }
    }
}
void MainServer::getMessagesSlot(QString sender, QString recipient){
    QSqlQuery queryMessage = QSqlQuery();
    queryMessage.prepare("SELECT * FROM messages WHERE (sender = :searchsender and recipient = :searchrecipient) or (sender = :searchrecipient and recipient = :searchsender) order by pk");
    queryMessage.bindValue(":searchsender", sender);
    queryMessage.bindValue(":searchrecipient", recipient);
    if(queryMessage.exec()) {
        while(queryMessage.next()) {
            QString message;
            QString forwardField = queryMessage.value(3).toString();
            QStringList forwards = forwardField.split(",");
            if (forwards.size()>1 || (forwards.size()==1 && !forwards.at(0).isEmpty())){
                for (QString item : forwards) {
                    message = "переслано от "+item+":"+message;
                }
                message.remove("{").remove("}");
            }
            message = queryMessage.value(1).toString()+":"+message+queryMessage.value(4).toString();
            for (Server *worker : clients) {
                if (worker->getUserName()==sender) worker->sendToClient(message+"\n");
            }
        }
    }
    else {
        QSqlError error = queryMessage.lastError();
        qDebug() << "Ошибка выполнения запроса: " << error.text();
    }
}
void MainServer::addMessage(QString sender, QString message){
    QString recipient = message.split(":").at(0);
    message.remove(0, message.indexOf(":")+1);
    QStringList forward;
    while (message.startsWith("переслано от")){
        message.remove(0, 13);
        forward.append(message.left(message.indexOf(":")));
        message.remove(0, message.indexOf(":")+1);
    }
    QSqlQuery queryMessage = QSqlQuery();
    if (!forward.isEmpty()) {
        queryMessage.prepare("INSERT INTO messages (sender, recipient, forwardedusers, message) VALUES (:addsender, :addrecipient, :addforwardedusers, :addmessage)");
        queryMessage.bindValue(":addforwardedusers", "{"+forward.join(",")+"}");
    }
    else queryMessage.prepare("INSERT INTO messages (sender, recipient, message) VALUES (:addsender, :addrecipient, :addmessage)");
    queryMessage.bindValue(":addsender", sender);
    queryMessage.bindValue(":addrecipient", recipient);
    queryMessage.bindValue(":addmessage", message);
    if(!queryMessage.exec()) {
        emit logMessage("Не удалось сохранить сообщение "+recipient+" от "+sender);
        if (log.isOpen())
        {
            QByteArray data = QString("Не удалось сохранить сообщение "+recipient+" от "+sender).toUtf8();
            log.write(data);
        }
    }
}
