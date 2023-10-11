#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QLabel>
#include <QHostInfo>

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
    void closeEvent(QCloseEvent *event);
private slots:
    // 自定义槽函数
    void onConnected();
    void onDisconnected();
    void onStateChanged(QAbstractSocket::SocketState socketState);
    void onReadyRead();

    void on_actConnect_triggered();

    void on_actDisconnect_triggered();

    void on_actClear_triggered();

    void on_btnSend_clicked();

private:
    QString getLocalIP();
private:
    Ui::MainWindow *ui;
    QTcpSocket* tcpClient;      // 通信用的套接字
    QLabel* labelSocketState;   // Socket 状态
};
#endif // MAINWINDOW_H
