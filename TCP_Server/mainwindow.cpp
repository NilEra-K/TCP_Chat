#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaEnum>
#include <QVariant>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 初始化状态栏
    labelListen = new QLabel("监听状态 : ");
    labelListen->setMinimumWidth(200);
    ui->statusbar->addWidget(labelListen);

    labelSocketState = new QLabel(" Socket 状态 : ");
    labelSocketState->setMinimumWidth(200);
    ui->statusbar->addWidget(labelSocketState);

    // 初始化按钮状态
    ui->actStart->setEnabled(true);
    ui->actStop->setEnabled(false);

    // 本机 IP
    initHost();
    QString localIP = getLocalIP();
    setWindowTitle(windowTitle() + " -- 本机IP : " + localIP);    // 设置窗口标题
    ui->cmbIP->addItem(localIP);

    // TCP 服务器
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {
    if(tcpServer->isListening()) {
        tcpServer->close(); // 停止监听
    }
    event->accept();
}

void MainWindow::initHost() {
    QString hostName = QHostInfo::localHostName();          // 本地主机名
    QHostInfo hostInfo = QHostInfo::fromName(hostName);     // 本地主机信息
    QString localIP = "";
    QList<QHostAddress> addrList = hostInfo.addresses();    // IP 地址列表
    if(!addrList.isEmpty()) {
        for(int i = 0; i < addrList.count(); i++) {
            QHostAddress addr = addrList.at(i);
            if(addr.protocol() == QAbstractSocket::IPv4Protocol) { // 只要得到本机的一个 IPv4 地址就直接退出循环
                localIP = addr.toString();
                ui->cmbIP->addItem(addr.toString(), QVariant(addr.toString()));
                // break;
            }
        }
    }

}

// 获取本机 IP
QString MainWindow::getLocalIP() {
//    QString hostName = QHostInfo::localHostName();          // 本地主机名
//    QHostInfo hostInfo = QHostInfo::fromName(hostName);     // 本地主机信息
//    QString localIP = "";
//    QList<QHostAddress> addrList = hostInfo.addresses();    // IP 地址列表
//    if(!addrList.isEmpty()) {
//        for(int i = 0; i < addrList.count(); i++) {
//            QHostAddress addr = addrList.at(i);
//            if(addr.protocol() == QAbstractSocket::IPv4Protocol) { // 只要得到本机的一个 IPv4 地址就直接退出循环
//                localIP = addr.toString();
//                // break;
//            }
//        }
//    }
    QString localIP = ui->cmbIP->currentText();
    return localIP;
}

void MainWindow::onNewConnection() {
    qDebug() << "onNewConnection() 已执行";
    // 将挂起的连接接受为已连接的 QTcpSocket, 获取一个用于通信的套接字 Socket
    tcpSocket = tcpServer->nextPendingConnection();
    // connect(tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
    // Qt 5.12 版本中出现 connected() 信号和槽函数连接失败的错误, 这里采用手动调用的方式
    onConnected();

    connect(tcpSocket, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    // stateChanged(QAbstractSocket::SocketState) 信号会在第一次失效, 因此也手动调用
    onStateChanged(tcpSocket->state());

    connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
}

void MainWindow::onConnected() { // 无实质性的功能, 只是在打印信息
    ui->plainTextEdit->appendPlainText(">>> Client Socket Connected...");
    // 当处于连接状态时, 获取匹配的 IP 地址/端口号(与服务器连接的客户端的 IP 地址/端口号)
    ui->plainTextEdit->appendPlainText(">>> Peer Address : " + tcpSocket->peerAddress().toString());
    ui->plainTextEdit->appendPlainText(">>> Peer Port : " + QString::number(tcpSocket->peerPort()));
}

void MainWindow::onDisconnected() {
    qDebug() << "执行 onDisconnected()";
    ui->plainTextEdit->appendPlainText(">>> Client Socket Disconnected...");
    // tcpSocket->deleteLater();   // 稍后删除, 或者直接删除
    delete tcpSocket;
    tcpSocket = nullptr;
}

void MainWindow::onStateChanged(QAbstractSocket::SocketState socketState) {
    // 直接显示当前状态
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    labelSocketState->setText("Socket状态 : " + QString(metaEnum.valueToKey(socketState)));
}

void MainWindow::onReadyRead() { // 说明缓冲区有待读取数据
    while (tcpSocket->canReadLine()) {
        ui->plainTextEdit->appendPlainText("[接收] " + tcpSocket->readLine());
    }
}

// 开始监听
void MainWindow::on_actStart_triggered() {
    QString ip = ui->cmbIP->currentText();  // IP 地址(字符串格式)
    QHostAddress addr(ip);                  // 使用构造函数将 字符型 IP 地址转化为 QHostAddress 型
    quint16 port = ui->spinPort->value();   // 端口
    tcpServer->listen(addr, port);          // 监听

    // 文本编辑框输出相关的信息
    ui->plainTextEdit->appendPlainText(">>> 开始监听...");
    ui->plainTextEdit->appendPlainText(">>> 服务器地址 : " + tcpServer->serverAddress().toString());
    ui->plainTextEdit->appendPlainText(">>> 服务器端口 : " + QString::number(tcpServer->serverPort()));

    // 更改按钮状态
    ui->actStart->setEnabled(false);
    ui->actStop->setEnabled(true);

    // 更改状态栏标签
    labelListen->setText("监听状态 : 正在监听...");
    // 监听成功后有连接(客户端)接入 => 发射 newConnection() 信号
}

// 停止监听
void MainWindow::on_actStop_triggered() {
    if(tcpServer->isListening()) { // 如果正处于监听状态
        tcpServer->close();        // 停止监听
        ui->actStart->setEnabled(true);
        ui->actStop->setEnabled(false);
        labelListen->setText("监听状态 : 已停止监听...");
    }
}

// 发送消息
void MainWindow::on_btnSend_clicked() {
    QString msg = ui->editMessage->text();
    ui->plainTextEdit->appendPlainText("[发送] " + msg);
    ui->editMessage->clear();
    ui->editMessage->setFocus();
    QByteArray str = msg.toUtf8();
    str.append("\n");
    tcpSocket->write(str);
}

// 清空文本框
void MainWindow::on_actTextClear_triggered() {
    ui->plainTextEdit->clear();
}

