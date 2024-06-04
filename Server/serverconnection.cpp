#include "serverconnection.h"

ServerConnection::ServerConnection(QObject *parent)
    : QObject(parent)
    , serverSocket(new QSslSocket(this))
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
    QFile certFile("org.crt");    //получение сертификата из файла
    if (!certFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open certificate file";
        return;
    }
    QSslCertificate sslCert(&certFile, QSsl::Pem);
    sslConfig.setLocalCertificate(sslCert);
    serverSocket->setSslConfiguration(sslConfig);
    connect(serverSocket, &QSslSocket::readyRead, this, &ServerConnection::read); //установка соединения для чтения сообщений
    connect(serverSocket, &QSslSocket::disconnected, this, &ServerConnection::disconnectFromClient);  //установка соединения для отключения клиента
    connect(serverSocket, SIGNAL(sslErrors(const QList<QSslError> &)), this, SLOT(handlingSslError(const QList<QSslError> &))); //обработка ошибок ssl соединения
}
bool ServerConnection::setSocketDescriptor(qintptr socketDescriptor)
{
    return serverSocket->setSocketDescriptor(socketDescriptor);
}
void ServerConnection::disconnectFromClient()    //слот отключения клиента
{
    emit sendEveryone("DISCONNECT", username);  //рассылка сообщения об отключении клиента от сервера
    emit disconnectedFromClient(this);  //отправка сигнала главному серверу об отключении клиента
    serverSocket->disconnectFromHost();
}

QString ServerConnection::getUserName()
{
    return username;
}
void ServerConnection::read()  //слот для чтения данных с сокета
{
    QByteArray jsonByteArray;
    QDataStream socketStream(serverSocket);
    while (!socketStream.atEnd()) {//пока при чтении не достигли конца
        socketStream >> jsonByteArray; //считывание данных с потока
        QJsonParseError parseError;
        const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonByteArray, &parseError);  //создание json-документа с проверкой на наличие ошибок
        if (parseError.error == QJsonParseError::NoError) { //если ошибок нет
            if (jsonDoc.isObject()){    //и json-документ можно преобразовать в json-объект
                QJsonObject jsonObject = jsonDoc.object();
                QJsonValue type = jsonObject.value("Type"); //получаем тип сообщения
                if (type.toString().compare("Login") == 0){   //если сообщение - логин нового пользователя
                    username = jsonObject.value("Login").toString();    //получаем логин
                    emit sendEveryone("CONNECT", username); //рассылка сообщения о новом пользователе
                    emit logMessage("Подключение нового пользователя: " + username);
                }
                if (type.toString().compare("Get Messages") == 0){   //если сообщение - запрос на получение сообщений диалога
                    //получаем логины двух пользователей
                    QString sender = jsonObject.value("Sender").toString();
                    QString recipient = jsonObject.value("Recipient").toString();
                    emit getMessages(sender, recipient);    //используем сигнал получения сообщений из базы данных
                }
                if (type.toString().compare("Message") == 0){   //если получено обычное сообщение
                    QString sender = jsonObject.value("Sender").toString();
                    QString recipient = jsonObject.value("Recipient").toString();
                    QString messageText = jsonObject.value("Message text").toString();
                    QString time = jsonObject.value("Time").toString();
                    emit searchClient(sender, recipient, messageText, time);    //используем сигнал для поиска получателя сообщения
                }
            }
        }
    }
}
void ServerConnection::sendToClient(QString sender, QString text, QString time){ //отправка сообщения клиенту
    QJsonObject message;    //формируем json-объект из типа сообщения, логина отправителя, текста сообщения и времени отправки
    message.insert("Type", QJsonValue::fromVariant("Message"));
    message.insert("Sender", QJsonValue::fromVariant(sender));
    message.insert("Message text", QJsonValue::fromVariant(text));
    message.insert("Time", QJsonValue::fromVariant(time));
    QDataStream serverStream(serverSocket);
    serverStream << QJsonDocument(message).toJson();    //отправка json
}
void ServerConnection::sendStatusToClient(QString status, QString user){  //отправка данных об изменении статуса пользователя
    QJsonObject message;    //формируем json-объект из типа сообщения, логина отправителя и статуса
    message.insert("Type", QJsonValue::fromVariant("Status"));
    message.insert("Status", QJsonValue::fromVariant(status));
    message.insert("Sender", QJsonValue::fromVariant(user));
    QDataStream serverStream(serverSocket);
    serverStream << QJsonDocument(message).toJson();    //отправка json
}
void ServerConnection::start(){
    serverSocket->startServerEncryption();
}
void ServerConnection::handlingSslError(const QList<QSslError>& errors) {   //слот обработки ошибок ssl
    for (const QSslError& error : errors) {
        if (error.error() != QSslError::SelfSignedCertificate && error.error() != QSslError::HostNameMismatch) {//если ошибка не связана с самоподписанным серификатом или названием узла
            qDebug() << "SSL error:" << error.error();  //выводим информацию об ошибке
            serverSocket->abort();  //разрыв соединения
        }
    }
    serverSocket->ignoreSslErrors();    //игнорируем некритические ошибки
}
