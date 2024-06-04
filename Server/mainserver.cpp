#include "mainserver.h"

MainServer::MainServer(QObject *parent)
    : QTcpServer(parent)
{
    //задаем ip-адрес и порт
    quint16 port = 2323;
    this->listen(QHostAddress::LocalHost, port);
    log.setFileName("log.txt"); //открываем файл для записи логов
    log.open(QIODevice::Append | QIODevice::Text);
    if (log.isOpen())
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "\nЗапуск сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
    udpServerSocket.bind(2323, QUdpSocket::ShareAddress);
    connect(&udpServerSocket, &QUdpSocket::readyRead, this, &MainServer::udpAnswer);    //устанавливаем соединение сигнала и слота для ответа на датаграммы клиентов
    //задаем параметры базы данных
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("Messages");
    db.setHostName("192.168.56.1");
    db.setUserName("postgres");
    db.setPassword("Postgres");
    if (!db.open() && log.isOpen()) {
        emit logMessage("Ошибка открытия базы данных:"+db.lastError().text());
        QByteArray data = QString("Ошибка открытия базы данных:"+db.lastError().text()+"\n").toUtf8();
        log.write(data);
    }
}

MainServer::~MainServer()
{
    for (ServerConnection *connection : clients) {
        connection->disconnectFromClient(); //отключение всех пользователей от сервера
    }
    if (log.isOpen())   //запись лога в файл
    {
        QDateTime currentDateTime = QDateTime::currentDateTime();
        QString str = "Отключение сервера: " + currentDateTime.toString("dd.MM.yyyy hh:mm:ss")+"\n";
        QByteArray data = str.toUtf8();
        log.write(data);
    }
    log.close();
}
void MainServer::incomingConnection(qintptr socketDescriptor)   //метод подключения клиента к серверу
{
    ServerConnection *connection = new ServerConnection(this);
    if (!connection->setSocketDescriptor(socketDescriptor)) {   //если дескриптор сокета не удалось задать
        connection->deleteLater();  //удаление подключения
        return;
    }
    //установка соединений сигналов объекта сервера для подключения клиентов со слотами основного сервера чата
    connect(connection, SIGNAL(sendEveryone(QString, QString)), this, SLOT(sendEveryone(QString, QString)));    //пересылка статуса всем пользователям
    connect(connection, SIGNAL(searchClient(QString, QString, QString, QString)), this, SLOT(searchClient(QString, QString, QString, QString)));    //поиск пользователя-получателя сообщения
    connect(connection, &ServerConnection::disconnectedFromClient, this, std::bind(&MainServer::disconnectClient, this, connection)); //отключение клиента
    connect(connection, SIGNAL(logMessage(QString)), this, SLOT(sendLogMessage(QString)));  //передача сообщения-лога
    connect(connection, SIGNAL(getMessages(QString, QString)), this, SLOT(getMessages(QString, QString)));   //получение сообщений из базы данных
    for (ServerConnection *worker : clients) {
        connection->sendStatusToClient("CONNECT", worker->getUserName()); //отправка новому пользователю списка всех подключенных клиентов
    }
    clients.append(connection); //добавление подключения в список
}

void MainServer::sendEveryone(QString status, QString message)    //слот отправки сообщений о статусе пользователя всем пользователям
{
    for (ServerConnection *worker : clients) {
        if (worker == sender()) //не отправляем сообщение отправителю
            continue;
        worker->sendStatusToClient(status, message);    //остальным - отправляем сообщение
    }
}

void MainServer::searchClient(QString sender, QString recipient, QString message, QString time){    //слот поиска клиента-получателя сообщения
    for (ServerConnection *worker : clients) {
        if (worker->getUserName()==recipient){  //если логин пользователя соответствует логину из сообщения
            addMessage(sender, recipient, message, time);   //сохраняем сообщение в базе данных
            worker->sendToClient(sender, message, time);    //отправляем сообщение пользователю
            emit logMessage("Отправка сообщения, отправитель: "+sender+", получатель: "+recipient);   //передача информации графическому интерфейсу
            if (log.isOpen())   //запись лога в файл
            {
                QByteArray data = QString("Отправка сообщения, отправитель: "+sender+", получатель: "+recipient+"\n").toUtf8();
                log.write(data);
            }
        }
    }
}
void MainServer::disconnectClient(ServerConnection* sender){  //слот отключения клиента от сервера
    emit logMessage("Отключение от сервера пользователя: "+sender->getUserName());   //передача информации графическому интерфейсу
    if (log.isOpen())   //запись лога в файл
    {
        QByteArray data = QString("Отключение от сервера пользователя: "+sender->getUserName()+"\n").toUtf8();
        log.write(data);
    }
    clients.removeAll(sender);  //удаление клиента из списка
}
void MainServer::sendLogMessage(QString message){   //слот отправки лога графическому интерфейсу
    emit logMessage(message);   //передача информации графическому интерфейсу
    if (log.isOpen())   //запись лога в файл
    {
        QByteArray data = QString(message+"\n").toUtf8();
        log.write(data);
    }
}
void MainServer::udpAnswer() {  //слот ответа на датаграмму клиента
    while (udpServerSocket.hasPendingDatagrams())   //пока в сокете есть датаграммы для чтения
    {
        QByteArray datagram;
        datagram.resize(udpServerSocket.pendingDatagramSize()); //задаем размер массива байтов равный размеру датаграммы
        QHostAddress sender;
        quint16 senderPort;
        udpServerSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);  //получение данных с датаграммы
        if (datagram=="BroadcastRequest"){  //если датаграмма пришла с тем же текстом, который отправил пользователь
            QByteArray senderAddress = sender.toString().toUtf8();
            udpServerSocket.writeDatagram(senderAddress, sender, senderPort);   //отправляем датаграмму с IP-адресом пользователя
        }
    }
}
void MainServer::getMessages(QString sender, QString recipient){    //слот получения сообщений из базы данных
    QSqlQuery queryMessage = QSqlQuery();   //создание запроса
    queryMessage.prepare("SELECT * FROM messages WHERE (sender = :searchsender and recipient = :searchrecipient) or (sender = :searchrecipient and recipient = :searchsender) order by pk");
    //передача значений в запрос
    queryMessage.bindValue(":searchsender", sender);
    queryMessage.bindValue(":searchrecipient", recipient);
    if(queryMessage.exec()) {  //если запрос успешно выполнился
        while(queryMessage.next()) {    //пока записи извлекаются
            QString message;
            QString forwardField = queryMessage.value(3).toString();
            QStringList forwards = forwardField.split(",");
            //формируем информацию о пересылке сообщения
            if (forwards.size()>1 || (forwards.size()==1 && !forwards.at(0).isEmpty())){
                for (QString item : forwards) {
                    message = "переслано от "+item+":\n"+message;
                }
                message.remove("{").remove("}");
            }
            message = message+queryMessage.value(4).toString(); //добавляем сам текст сообщения
            //получаем время в необходимом формате
            QString time = QDateTime::fromString(queryMessage.value(5).toString(), "yyyy-MM-ddThh:mm:ss.zzz").toString("dd.MM.yyyy hh:mm");
            for (ServerConnection *worker : clients) {
                //отправляем сообщение необходимому клиенту
                if (worker->getUserName()==sender) worker->sendToClient(queryMessage.value(1).toString(), message, time);
            }
        }
    }
    else {
        QSqlError error = queryMessage.lastError();
        qDebug() << "Ошибка выполнения запроса: " << error.text();
    }
}
void MainServer::addMessage(QString sender, QString recipient, QString message, QString time){  //добавление сообщения в базу данных
    QStringList forward;
    while (message.startsWith("переслано от")){ //получения списка пользователей, от кого пересылалось сообщение
        message.remove(0, 13);
        forward.append(message.left(message.indexOf(":")));
        message.remove(0, message.indexOf("\n")+1);
    }
    QSqlQuery queryMessage = QSqlQuery();
    if (!forward.isEmpty()) {   //создание запроса для пересылаемого сообщения
        queryMessage.prepare("INSERT INTO messages (sender, recipient, forwardedusers, message, time) VALUES (:addsender, :addrecipient, :addforwardedusers, :addmessage, :addtime)");
        queryMessage.bindValue(":addforwardedusers", "{"+forward.join(",")+"}");
    }
    //создание запроса для сообщения, которое не пересылалось
    else queryMessage.prepare("INSERT INTO messages (sender, recipient, message, time) VALUES (:addsender, :addrecipient, :addmessage, :addtime)");
    //передача значений в запрос
    queryMessage.bindValue(":addsender", sender);
    queryMessage.bindValue(":addrecipient", recipient);
    queryMessage.bindValue(":addmessage", message);
    queryMessage.bindValue(":addtime", QDateTime::fromString(time, "dd.MM.yyyy hh:mm"));
    if(!queryMessage.exec()) {  //если запрос успешно выполнился
        emit logMessage("Не удалось сохранить сообщение " + recipient + " от " + sender);
        if (log.isOpen())
        {
            QByteArray data = QString("Не удалось сохранить сообщение " + recipient + " от " + sender).toUtf8();
            log.write(data);
        }
    }
}
