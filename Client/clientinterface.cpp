#include "clientinterface.h"
#include "ui_clientinterface.h"

ClientInterface::ClientInterface(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientInterface)   //создаем новый объект графического интерфейса
    , chatClient(new Client(this))    //создаем новый объект логики клиента
    , chatModel(new QStandardItemModel(this)) //создаем новый объект модели
{
    ui->setupUi(this);
    chatModel->insertColumn(0); //вставка столбца модели чата
    ui->chat->setModel(chatModel);    //модель используется для listView
    //изначально отключаем все элементы, кроме кнопки подключения
    ui->chat->setEnabled(false);
    ui->send->setEnabled(false);
    ui->message->setEnabled(false);
    //установка соединения сигналов объекта логики со слотами объекта интерфейса
    connect(chatClient, &Client::connected, this, &ClientInterface::connectedToServer);   //соединение с сервером
    connect(chatClient, &Client::messageReceived, this, &ClientInterface::messageReceived);   //получение сообщения
    connect(chatClient, &Client::disconnected, this, &ClientInterface::disconnectedFromServer);   //отключение от сервера
    //установка соединения сигналов элементов окна со слотами объекта интерфейса
    connect(ui->send, &QPushButton::clicked, this, &ClientInterface::sendMessage);  //отправка сообщений по нажатию кнопки
    connect(ui->message, &QLineEdit::returnPressed, this, &ClientInterface::sendMessage);  //отправка сообщений по нажатию enter в lineEdit
    connect(ui->connect, &QPushButton::clicked, this, &ClientInterface::attemptConnection); //подключение к серверу по нажатию кнопки
}
ClientInterface::~ClientInterface()
{
    delete ui;
}
void ClientInterface::attemptConnection()   //слот вызова подключения к серверу
{
    chatClient->connectToServer();
}
void ClientInterface::connectedToServer()   //слот подключения к серверу
{
    QString newUsername;
    do {
        newUsername = QInputDialog::getText(this, tr("Введите логин"), tr("Логин: "));  //ввод в диалоговом окне логина пользователя
        if (newUsername.isEmpty()) QMessageBox::warning(nullptr, "Предупреждение", "Имя не было введено!");
    } while (newUsername.isEmpty());    //пока логин не будет введен
    chatClient->sendLogin(newUsername);   //отправка логина на сервер
    lastUserName = newUsername;
    //разблокировка элементов окна
    ui->send->setEnabled(true);
    ui->message->setEnabled(true);
    ui->chat->setEnabled(true);
    lastUserName.clear();
}

void ClientInterface::messageReceived(QString sender, QString text)   //слот получения сообщения
{
    int newRow = chatModel->rowCount();   //сохранение количества строк в чате
    if (lastUserName != sender) //если отправитель сообщения изменился
        lastUserName = sender;    //меняем логин последнего отправителя
    QFont boldFont;
    boldFont.setBold(true); //задаем жирный шрифт для вывода отправителя
    chatModel->insertRows(newRow, 2); //добавление строк в выводе чата
    chatModel->setData(chatModel->index(newRow, 0), sender + ':');  //вывод отправителя
    chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);  //выравнивание текста
    chatModel->setData(chatModel->index(newRow, 0), boldFont, Qt::FontRole);    //задаем жирный шрифт
    ++newRow;   //переход на новую строку
    //вывод текста сообщения
    chatModel->setData(chatModel->index(newRow, 0), text);
    chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->chat->scrollToBottom();
}

void ClientInterface::sendMessage() //слот отправки сообщения
{
    chatClient->sendMessage(ui->message->text()); //вызов слота отправки из объекта логики
    const int newRow = chatModel->rowCount();   //сохранение количества строк в чате
    //вывод текста отправленного сообщения
    chatModel->insertRow(newRow);
    chatModel->setData(chatModel->index(newRow, 0), ui->message->text());
    chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    ui->message->clear();
    ui->chat->scrollToBottom();
    lastUserName.clear();
}

void ClientInterface::disconnectedFromServer() //слот отключения от сервера
{
    QMessageBox::warning(this, tr("Отключено"), tr("Соединение с сервером прервано"));
    //отключаем все элементы, кроме кнопки подключения
    ui->send->setEnabled(false);
    ui->message->setEnabled(false);
    ui->chat->setEnabled(false);
    ui->connect->setEnabled(true);
    lastUserName.clear();
}


