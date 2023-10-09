#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QHostInfo>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event);
private:
    QString getLocalIP();   // 获取本机 IP 地址

private slots:
    // 当服务器有新连接接入的时候就会发送 newConnection() 信号, onNewConnection() 是对该信号的处理
    void onNewConnection();

    void onConnected();
    void onDisconnected();
    void stateChanged(QAbstractSocket::SocketState socketState);

    // 当有数据进入时会发送 readyRead() 信号
    void onReadyRead();

private:
    Ui::MainWindow *ui;

    QLabel* labelListen;        // 状态栏显示监听状态
    QLabel* labelSocketState;   // 状态栏显示 Socket 状态
    QTcpServer* tcpServer;      // TCP 服务器
    QTcpSocket* tcpSocket;      // TCP 通信用的 Socket
};
#endif // MAINWINDOW_H
