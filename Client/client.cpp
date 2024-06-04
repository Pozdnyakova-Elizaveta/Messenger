#include "client.h"

Client::Client(QObject *parent)
    : QObject(parent)
    , clientSocket(new QSslSocket(this))
    , udpClientSocket(new QUdpSocket(this))
{
    QSslConfiguration sslConfig;    //конфигурация ssl соединения
    sslConfig.setProtocol(QSsl::TlsV1_2);    //задаем протокол TLS 1.2
    sslConfig.setPeerVerifyMode(QSslSocket::VerifyPeer);    //режим проверки - по действительному сертификату
    QFile keyFile("rootCA.key");    //получение приватного ключа из файла
    if (!keyFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open key file";
        return;
    }
    QSslKey privateKey(&keyFile, QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
    sslConfig.setPrivateKey(privateKey);
    QFile certFile("client.crt");    //получение сертификата из файла
    if (!certFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open certificate file";
        return;
    }
    QSslCertificate sslCert(&certFile, QSsl::Pem);
    sslConfig.setLocalCertificate(sslCert);
    clientSocket->setSslConfiguration(sslConfig);
    //установка соединения сигнала и слота
    connect(clientSocket, &QSslSocket::connected, this, &Client::connected);   //подключение клиента к серверу
    connect(clientSocket, &QSslSocket::disconnected, this, &Client::disconnected);  //отключение клиента от сервера
    connect(clientSocket, &QSslSocket::readyRead, this, &Client::read);    //получение данных с сокета
    connect(clientSocket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(handlingSslError(const QList<QSslError> &))); //обработка ошибок ssl соединения
    connect(udpClientSocket, &QUdpSocket::readyRead, this, &Client::readDatagram);   //чтение датаграммы
}
void Client::sendLogin(QString userName){   //отправка логина на сервер
    QJsonObject message;    //формирование json-объекта из типа сообщения и логина
    message.insert("Type", QJsonValue::fromVariant("Login"));
    message.insert("Login", QJsonValue::fromVariant(userName));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();    //отправка json
}
void Client::sendMessageRequest(QString sender, QString recipient){   //отправка запроса на получение сообщений с базы данных
    QJsonObject message;    //формирование json-объекта из типа сообщения, логина отправителя и логина получателя
    message.insert("Type", QJsonValue::fromVariant("Get Messages"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Recipient", QJsonValue::fromVariant(recipient));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();    //отправка json
}

void Client::sendMessage(QString sender, QString recipient, QString text, QString time)   //отправка сообщения одного пользователю другому
{
    QJsonObject message;    //формирование json-объекта из типа сообщения, логина отправителя, логина получателя, текста сообщения и времени отправки
    message.insert("Type", QJsonValue::fromVariant("Message"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Recipient", QJsonValue::fromVariant(recipient));
    message.insert("Message text", QJsonValue::fromVariant(text));
    message.insert("Time", QJsonValue::fromVariant(time));
    QDataStream clientStream(clientSocket);
    clientStream << QJsonDocument(message).toJson();    //отправка json
}

void Client::readDatagram()     //чтение датаграммы
{
    while (udpClientSocket->hasPendingDatagrams())  //пока в сокете есть датаграммы для чтения
    {
        QByteArray datagram;
        datagram.resize(udpClientSocket->pendingDatagramSize());    //задаем размер массива байтов равный размеру датаграммы
        QHostAddress sender;
        quint16 senderPort;
        udpClientSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);  //получение данных с датаграммы
        QRegularExpression reg("[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}");   //выражение для проверки, является ли строка ip-адресом
        if (reg.match(datagram).hasMatch()){ //если ip-адрес содержится в датаграмме
            connectToServer(datagram, sender.toString(), senderPort);   //выполнение подключения к серверу
        }
    }
}
void Client::connectToServer(QString IPServer, QString IPClient, quint16 port){ //подключение к серверу по полученному IP-адресу
    if (IPServer==IPClient) clientSocket->connectToHostEncrypted("127.0.0.1", port);  //ip-адреса сервера и клиента равны - сервер и клиент на одном компьютере, адрес - localhost
    else clientSocket->connectToHostEncrypted(IPServer, port);    //иначе - подключение по полученному адресу сервера
}
void Client::sendDatagram(){    //отправка датаграммы широковещательным запросом
    QByteArray datagram = "BroadcastRequest";
    udpClientSocket->writeDatagram(datagram, QHostAddress::Broadcast, 2323);
}
void Client::read() //слот для чтения данных с сокета
{
    QByteArray jsonByteArray;
    QDataStream socketStream(clientSocket);
    while (!socketStream.atEnd()) { //пока при чтении не достигли конца
        socketStream >> jsonByteArray; //считывание данных с потока
        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonByteArray, &parseError);  //создание json-документа с проверкой на наличие ошибок
        if (parseError.error == QJsonParseError::NoError) { //если ошибок нет
            if (jsonDoc.isObject()){    //и json-документ можно преобразовать в json-объект
                QJsonObject jsonObject = jsonDoc.object();
                QJsonValue type = jsonObject.value("Type"); //получение типа сообщения
                if (type.toString().compare("Status") == 0){  //если это сообщение о изменении статуса пользователя
                    QString sender = jsonObject.value("Sender").toString(); //получение отправителя
                    QString status = jsonObject.value("Status").toString(); //получение статуса
                    emit statusReceived(status, sender);    //отправка сигнала об изменении статуса пользователя
                }
                if (type.toString().compare("Message") == 0){    //если это сообщение одного пользователя другому
                    QString sender = jsonObject.value("Sender").toString();
                    QString text = jsonObject.value("Message text").toString();
                    QString time = jsonObject.value("Time").toString();
                    emit messageReceived(sender, text, time);   //отправка сигнала c данными сообщения
                }
            }
        }
    }
}
void Client::handlingSslError(const QList<QSslError>& errors) { //слот обработки ошибок ssl
    for (const QSslError& error : errors) {
        if (error.error() != QSslError::SelfSignedCertificate && error.error() != QSslError::HostNameMismatch) {//если ошибка не связана с самоподписанным серификатом или названием узла
            qDebug() << "SSL error:" << error.error();  //выводим информацию об ошибке
            clientSocket->abort();  //разрыв соединения
        }
    }
    clientSocket->ignoreSslErrors();    //игнорируем некритические ошибки
}
