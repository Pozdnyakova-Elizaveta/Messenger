#include "clientwindow.h"
#include "ui_clientwindow.h"

ClientWindow::ClientWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ClientWindow)
    , chatClient(new Client(this))
    , chatModel(new QStandardItemModel(this))
{
    ui->setupUi(this);
    chatModel->insertColumn(0); //вставка столбца модели чата
    ui->chat->setModel(chatModel);    //модель используется для listView
    //изначально отключаем все элементы, кроме кнопки подключения
    ui->chat->setEnabled(false);
    ui->send->setEnabled(false);
    ui->message->setEnabled(false);
    ui->userBox->addItem("");
    //установка соединения сигналов объекта логики со слотами объекта интерфейса
    connect(chatClient, &Client::connected, this, &ClientWindow::login);   //вход в систему
    connect(chatClient, &Client::messageReceived, this, &ClientWindow::messageReceived);   //получение сообщения пользователя
    connect(chatClient, &Client::statusReceived, this, &ClientWindow::statusReceived);   //получение сообщения о смене статуса пользователя
    connect(chatClient, &Client::disconnected, this, &ClientWindow::disconnectedFromServer);   //отключение от сервера
    //установка соединения сигналов элементов окна со слотами объекта интерфейса
    connect(ui->send, &QPushButton::clicked, this, &ClientWindow::sendMessage);  //отправка сообщений по нажатию кнопки
    connect(ui->message, &QLineEdit::returnPressed, this, &ClientWindow::sendMessage);  //отправка сообщений по нажатию enter в lineEdit
    connect(ui->connect, &QPushButton::clicked, this, &ClientWindow::connected); //поиск сервера по нажатию кнопки
    connect(ui->userBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ClientWindow::updateChat);  //обновление чата при выборе другого пользователя из списка
    connect(ui->chat, SIGNAL(clicked(QModelIndex)), this, SLOT(openForwardMenu(QModelIndex)));   //при нажатии на сообщение - переход к меню пересылки сообщения
    connect(&forwardingMenu, &QMenu::triggered, this, &ClientWindow::forwardMessage);  //при выборе в меню пользователя для пересылки - пересылка сообщения
}
ClientWindow::~ClientWindow()
{
    delete ui;
}
void ClientWindow::connected()   //слот вызова подключения к серверу
{
    chatClient->connectToServer();
}
void ClientWindow::login()   //слот входа в систему
{
    QString newUsername;
    do {
        newUsername = QInputDialog::getText(this, tr("Введите логин"), tr("Логин: "));  //ввод в диалоговом окне логина пользователя
        if (newUsername.isNull()) { //если нажата кнопка "Отмена"
            chatClient->disconnected();
            return;
        }
        if (newUsername.isEmpty()) QMessageBox::warning(nullptr, "Предупреждение", "Имя не было введено!");
        else {
            for (int i=0; i!=ui->userBox->count(); i++){
                if (newUsername==ui->userBox->itemText(i)){
                    newUsername.clear();
                    QMessageBox::warning(nullptr, "Предупреждение", "Такой пользователь уже в сети!");
                }
            }
        }
    } while (newUsername.isEmpty());    //пока логин не будет введен
    username = newUsername;
    chatClient->sendLogin(newUsername);   //отправка логина на сервер
    ui->userName->setText(username);
    ui->userName->setAlignment(Qt::AlignCenter);
    //разблокировка элементов окна
    ui->send->setEnabled(true);
    ui->message->setEnabled(true);
    ui->chat->setEnabled(true);
    ui->connect->setEnabled(false);
    ui->connect->setVisible(false);
}

void ClientWindow::messageReceived(QString sender, QString text, QString time)   //слот получения сообщения
{
    //вывод сообщения в зависимости от его отправителя
    if (ui->userBox->currentText() == sender || sender == username){
        if (sender!=username) outputMessage(sender + ":\n" + text + "\n" + time, Qt::AlignLeft);
        else outputMessage(sender + ":\n" + text + "\n" + time, Qt::AlignRight);
    }
}
void ClientWindow::statusReceived(QString status, QString user){    //слот получения данных об изменении статуса пользователя
    if (status == "CONNECT")
        ui->userBox->addItem(user); //добавление пользователя в список, если он подключился
    if (status == "DISCONNECT")
        ui->userBox->removeItem(ui->userBox->findText(user));   //удаление пользователя, если он отключился
}
void ClientWindow::sendMessage() //слот отправки сообщения
{
    if (ui->userBox->count() == 0) return;
    QString time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm");   //получение текущей даты и времени
    chatClient->sendMessage(username, ui->userBox->currentText(), ui->message->text(), time); //вызов отправки сообщения из объекта логики
    QString message = username + ":\n" + ui->message->text() + "\n" + time;
    outputMessage(message, Qt::AlignRight); //вывод отправленного сообщения
    ui->message->clear();
    ui->chat->scrollToBottom();
}

void ClientWindow::disconnectedFromServer() //слот отключения от сервера
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
void ClientWindow::updateChat(){    //слот изменения чата
    chatModel->removeRows(0, chatModel->rowCount());    //очистка вывода чата
    if (!ui->userBox->currentText().isEmpty() && !username.isEmpty())
        chatClient->sendMessageRequest(username, ui->userBox->currentText());   //запрос на получение сообщений с выбранным пользователем
}
void ClientWindow::openForwardMenu(QModelIndex index){  //слот открытия меню пересылки
    forwardMessageText = index.data(Qt::DisplayRole).toString();    //сохраняем пересылаемое сообщение
    forwardMessageText = forwardMessageText.left(forwardMessageText.lastIndexOf('\n')); //удаляем из него старые дату и время
    forwardingMenu.clear();
    QMenu *submenu = forwardingMenu.addMenu("Переслать пользователю:");
    for (int i=0; i!=ui->userBox->count(); i++){    //создаем список пользователей для пересылки на основе списка userBox
        if (!ui->userBox->itemText(i).isEmpty()) submenu->addAction(ui->userBox->itemText(i));
    }
    forwardingMenu.exec(QCursor::pos());
}
void ClientWindow::forwardMessage(QAction *action){ //слот пересылки сообщения
    QString message = "переслано от " + forwardMessageText;
    QString time = QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm");
    chatClient->sendMessage(username, action->text(), message, time);   //отправка сообщения
    outputMessage(username + ":\n" + message + "\n" + time, Qt::AlignRight);    //вывод сообщения
}
void ClientWindow::outputMessage(QString message, Qt::AlignmentFlag flag){  //вывод сообщения
    int newRow = chatModel->rowCount();   //сохранение количества строк в чате
    chatModel->insertRows(newRow, 1);
    chatModel->setData(chatModel->index(newRow, 0), message);   //добавления текста сообщения
    chatModel->setData(chatModel->index(newRow, 0), int(flag | Qt::AlignVCenter), Qt::TextAlignmentRole);
    //блокировка изменения текста в окне
    auto item = chatModel->item(newRow, 0);
    item->setFlags(item->flags() &= ~Qt::ItemIsEditable);
}
