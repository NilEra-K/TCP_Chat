#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMetaEnum>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 状态栏
    labelSocketState = new QLabel("Socket状态 : ");
    labelSocketState->setMinimumWidth(300);
    ui->statusbar->addWidget(labelSocketState);

    // 本机 IP
    initHost();
    QString localIP = getLocalIP();
    setWindowTitle(windowTitle() + "--本机IP : " + localIP);
    ui->cmbServerIP->addItem(localIP);

    // Socket
    tcpClient = new QTcpSocket(this);       // 创建 QTcpSocket 对象
    connect(tcpClient, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(tcpClient, SIGNAL(stateChanged(QAbstractSocket::SocketState)), this, SLOT(onStateChanged(QAbstractSocket::SocketState)));
    connect(tcpClient, SIGNAL(readyRead()), this, SLOT(onReadyRead()));
    connect(tcpClient, SIGNAL(disconnected()), this, SLOT(onDisconnected()));

    // 初始化按钮状态
    ui->actConnect->setEnabled(true);
    ui->actDisconnect->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
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
                ui->cmbServerIP->addItem(addr.toString(), QVariant(addr.toString()));
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
    QString localIP = ui->cmbServerIP->currentText();
    return localIP;
}

void MainWindow::onConnected(){
    ui->plainTextEdit->appendPlainText(">>> 已连接到服务器");
    ui->plainTextEdit->appendPlainText(">>> Peer Address : " + tcpClient->peerAddress().toString());
    ui->plainTextEdit->appendPlainText(">>> Peer Port : " + QString::number(tcpClient->peerPort()));
    ui->actConnect->setEnabled(false);
    ui->actDisconnect->setEnabled(true);
}

void MainWindow::onDisconnected() {
    ui->plainTextEdit->appendPlainText(">>> 已断开服务器连接...");
    ui->actConnect->setEnabled(true);
    ui->actDisconnect->setEnabled(false);
}

void MainWindow::onStateChanged(QAbstractSocket::SocketState socketState) {
    // 直接显示当前状态
    QMetaEnum metaEnum = QMetaEnum::fromType<QAbstractSocket::SocketState>();
    labelSocketState->setText("Socket状态 : " + QString(metaEnum.valueToKey(socketState)));
}

void MainWindow::onReadyRead() {
    while(tcpClient->canReadLine()) {
        ui->plainTextEdit->appendPlainText("[接收] " + tcpClient->readLine());
    }
}

// 连接到服务器
void MainWindow::on_actConnect_triggered() {
    QString hostName = ui->cmbServerIP->currentText();
    quint16 port = ui->spinServerPort->value();
    tcpClient->connectToHost(hostName, port);
}

// 断开连接, 会发送 disconnected() 信号
void MainWindow::on_actDisconnect_triggered() {
    if(tcpClient->state() == QAbstractSocket::ConnectedState) {
        tcpClient->disconnectFromHost();
    }
}

// 清空文本框
void MainWindow::on_actClear_triggered() {
    ui->plainTextEdit->clear();
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if(tcpClient->state() == QAbstractSocket::ConnectedState) {
        tcpClient->disconnectFromHost();
    }
    event->accept();
}

// 发送信息
void MainWindow::on_btnSend_clicked() {
    QString msg = ui->editMessage->text();
    ui->plainTextEdit->appendPlainText("[发送] " + msg);
    ui->editMessage->clear();
    ui->editMessage->setFocus();    // 将光标重新放会 editMessage 控件

    QByteArray str = msg.toUtf8();
    str.append("\n");
    tcpClient->write(str);
}

