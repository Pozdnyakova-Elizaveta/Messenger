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
    connect(ui->userBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClientInterface::clearChat);
    connect(ui->chat, SIGNAL(clicked(QModelIndex)), this, SLOT(forwardMessage(QModelIndex)));
    connect(&menu, &QMenu::triggered, this, &ClientInterface::menuActivated);
}
ClientInterface::~ClientInterface()
{
    delete ui;
}
void ClientInterface::attemptConnection()   //слот вызова подключения к серверу
{
    chatClient->sendDatagram();
}
void ClientInterface::connectedToServer()   //слот подключения к серверу
{
    QString newUsername;
    do {
        newUsername = QInputDialog::getText(this, tr("Введите логин"), tr("Логин: "));  //ввод в диалоговом окне логина пользователя
        if (newUsername.isEmpty()) QMessageBox::warning(nullptr, "Предупреждение", "Имя не было введено!");
    } while (newUsername.isEmpty());    //пока логин не будет введен
    name = newUsername;
    chatClient->sendLogin(newUsername);   //отправка логина на сервер
    ui->userName->setText(name);
    //разблокировка элементов окна
    ui->send->setEnabled(true);
    ui->message->setEnabled(true);
    ui->chat->setEnabled(true);
    ui->connect->setEnabled(false);
    ui->connect->setVisible(false);
}

void ClientInterface::messageReceived(QString text)   //слот получения сообщения
{
    int index = text.indexOf(":");  //индекс разделения логина отправителя и сообщения
    if (text.split(":").at(0)=="CONNECT")
        ui->userBox->addItem(text.remove(0, index+1));
    else if (text.split(":").at(0)=="DISCONNECT")
        ui->userBox->removeItem(ui->userBox->findText(text.remove(0, index+1)));
    else{
        QString sender = text.split(":").at(0);
        if (nameSender!=sender && !nameSender.isEmpty()){
            clearChat();
        }
        nameSender = sender;
        int newRow = chatModel->rowCount();   //сохранение количества строк в чате
        chatModel->insertRows(newRow, 1);
        text.replace(":",":\n");
        chatModel->setData(chatModel->index(newRow, 0), text);
        chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignLeft | Qt::AlignVCenter), Qt::TextAlignmentRole);
        auto item = chatModel->item(newRow, 0);
        item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
        ui->chat->scrollToBottom();
    }
}

void ClientInterface::sendMessage() //слот отправки сообщения
{
    if (ui->userBox->count()==0) return;
    chatClient->sendMessage(ui->userBox->currentText()+":"+ui->message->text()); //вызов слота отправки из объекта логики
    int newRow = chatModel->rowCount();   //сохранение количества строк в чате
    chatModel->insertRows(newRow, 1);
    chatModel->setData(chatModel->index(newRow, 0), name + ":\n" +ui->message->text());
    chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    auto item = chatModel->item(newRow, 0);
    item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
    ui->message->clear();
    ui->chat->scrollToBottom();
}

void ClientInterface::disconnectedFromServer() //слот отключения от сервера
{
    QMessageBox::warning(this, tr("Отключено"), tr("Соединение с сервером прервано"));
    //отключаем все элементы, кроме кнопки подключения
    ui->send->setEnabled(false);
    ui->message->setEnabled(false);
    ui->chat->setEnabled(false);
    ui->connect->setEnabled(true);
    ui->connect->setVisible(true);
    ui->userBox->setEnabled(false);
}
void ClientInterface::clearChat(){
    chatModel->removeRows(0, chatModel->rowCount());
}
void ClientInterface::forwardMessage(QModelIndex index){
    recipientUser = index.data(Qt::DisplayRole).toString();
    recipientUser.replace("\n","");
    menu.clear();
    QMenu *recentFilesMenu = menu.addMenu("Переслать пользователю:");
    for (int i=0; i!=ui->userBox->count(); i++){
        recentFilesMenu->addAction(ui->userBox->itemText(i));
    }
    menu.exec(QCursor::pos());
}
void ClientInterface::menuActivated(QAction *action){
    QString message = ":переслано от "+recipientUser;
    chatClient->sendMessage(action->text()+message);
    message.replace(":",":\n");
    if (action->text()!=nameSender){
        clearChat();
        nameSender = action->text();
    }
    int newRow = chatModel->rowCount();   //сохранение количества строк в чате
    chatModel->insertRows(newRow, 1);
    chatModel->setData(chatModel->index(newRow, 0), name+message);
    chatModel->setData(chatModel->index(newRow, 0), int(Qt::AlignRight | Qt::AlignVCenter), Qt::TextAlignmentRole);
    auto item = chatModel->item(newRow, 0);
    item->setFlags(item->flags() &= ~Qt::ItemIsEditable);

}
