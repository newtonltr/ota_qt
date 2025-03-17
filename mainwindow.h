#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_socket_send_clicked();
    void on_pushButton_socket_connect_clicked();
    void on_pushButton_socket_disconnect_clicked();
    
    // TCP Socket槽函数
    void onSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead();
    void onSocketError(QAbstractSocket::SocketError socketError);

private:
    Ui::MainWindow *ui;
    QTcpSocket *tcpSocket;
    bool userInitiatedDisconnect; // 添加标志位，用于区分用户主动断开和服务器主动断开
    
    // 辅助函数
    QString getIpAddress();
    int getPort();
    QByteArray convertToHex(const QString &text);
};
#endif // MAINWINDOW_H
