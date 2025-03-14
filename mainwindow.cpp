#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QMessageBox>
#include <QNetworkProxy>  // 添加代理头文件

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , tcpSocket(nullptr)
{
    ui->setupUi(this);
    
    // 初始化TCP Socket
    tcpSocket = new QTcpSocket(this);
    
    // 设置不使用代理
    tcpSocket->setProxy(QNetworkProxy::NoProxy);
    
    // 连接TCP Socket信号和槽
    connect(tcpSocket, &QTcpSocket::connected, this, &MainWindow::onSocketConnected);
    connect(tcpSocket, &QTcpSocket::disconnected, this, &MainWindow::onSocketDisconnected);
    connect(tcpSocket, &QTcpSocket::readyRead, this, &MainWindow::onSocketReadyRead);
    connect(tcpSocket, &QTcpSocket::errorOccurred, this, &MainWindow::onSocketError);
    
    // 设置初始连接状态
    ui->label_connect_status->setText("未连接");
    ui->label_connect_status->setStyleSheet("color: black;");
    
    // 设置默认IP地址和端口号
    ui->textEdit_ip0->setText("192");
    ui->textEdit_ip1->setText("168");
    ui->textEdit_ip2->setText("0");
    ui->textEdit_ip3->setText("200");
    ui->textEdit_port->setText("7000");
}

MainWindow::~MainWindow()
{
    if (tcpSocket && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    }
    delete ui;
}

void MainWindow::on_pushButton_socket_send_clicked()
{
    if (!tcpSocket || tcpSocket->state() != QAbstractSocket::ConnectedState) {
        QMessageBox::warning(this, "警告", "未连接到服务器");
        return;
    }
    
    QString text = ui->textEdit_socket_input->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "警告", "发送内容不能为空");
        return;
    }
    
    QByteArray data;
    // 根据选择的格式发送数据
    if (ui->comboBox_send_format->currentText() == "char") {
        data = text.toUtf8();
    } else if (ui->comboBox_send_format->currentText() == "hex") {
        data = convertToHex(text);
    }
    
    // 发送数据
    tcpSocket->write(data);
}

void MainWindow::on_pushButton_socket_connect_clicked()
{
    // 检查当前连接状态
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        QMessageBox::information(this, "提示", "已连接到服务器");
        return;
    }
    
    // 如果正在连接中，提示用户并更新状态标签
    if (tcpSocket->state() == QAbstractSocket::ConnectingState) {
        QMessageBox::information(this, "提示", "正在连接中，请稍候...");
        ui->label_connect_status->setText("连接中");
        ui->label_connect_status->setStyleSheet("color: blue;");
        return;
    }
    
    // 检查IP地址输入框是否为空
    if (ui->textEdit_ip0->toPlainText().isEmpty() || 
        ui->textEdit_ip1->toPlainText().isEmpty() || 
        ui->textEdit_ip2->toPlainText().isEmpty() || 
        ui->textEdit_ip3->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "警告", "IP地址不能为空");
        return;
    }
    
    // 检查端口输入框是否为空
    if (ui->textEdit_port->toPlainText().isEmpty()) {
        QMessageBox::warning(this, "警告", "端口号不能为空");
        return;
    }
    
    QString ipAddress = getIpAddress();
    int port = getPort();
    
    if (ipAddress.isEmpty() || port <= 0) {
        QMessageBox::warning(this, "警告", "IP地址或端口号无效");
        return;
    }
    
    // 确保不使用代理
    tcpSocket->setProxy(QNetworkProxy::NoProxy);
    
    // 设置连接超时时间为10秒
    tcpSocket->setSocketOption(QAbstractSocket::LowDelayOption, 1);
    tcpSocket->setSocketOption(QAbstractSocket::KeepAliveOption, 1);
    
    // 更新状态为连接中
    ui->label_connect_status->setText("连接中");
    ui->label_connect_status->setStyleSheet("color: blue;");
    ui->textBrowser_recv_display->append("正在连接到服务器...");
    
    // 连接到服务器
    tcpSocket->connectToHost(ipAddress, port);
}

void MainWindow::on_pushButton_socket_disconnect_clicked()
{
    if (tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->disconnectFromHost();
    } else {
        QMessageBox::information(this, "提示", "未连接到服务器");
    }
}

void MainWindow::onSocketConnected()
{
    ui->textBrowser_recv_display->append("已连接到服务器");
    
    // 更新连接状态标签
    ui->label_connect_status->setText("已连接");
    ui->label_connect_status->setStyleSheet("color: red;");
}

void MainWindow::onSocketDisconnected()
{
    ui->textBrowser_recv_display->append("已断开连接");
    
    // 更新连接状态标签
    ui->label_connect_status->setText("未连接");
    ui->label_connect_status->setStyleSheet("color: black;");
}

void MainWindow::onSocketReadyRead()
{
    QByteArray data = tcpSocket->readAll();
    
    // 根据选择的格式显示数据
    if (ui->comboBox_display_format->currentText() == "char") {
        ui->textBrowser_recv_display->append("接收: " + QString::fromUtf8(data));
    } else if (ui->comboBox_display_format->currentText() == "hex") {
        QString hexString;
        for (int i = 0; i < data.size(); ++i) {
            hexString += QString("%1 ").arg((quint8)data.at(i), 2, 16, QChar('0')).toUpper();
        }
        ui->textBrowser_recv_display->append("接收(HEX): " + hexString);
    }
}

void MainWindow::onSocketError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    
    // 如果是超时错误，特别处理
    if (tcpSocket->error() == QAbstractSocket::SocketTimeoutError) {
        ui->textBrowser_recv_display->append("错误: 连接超时(10秒)");
    } else {
        ui->textBrowser_recv_display->append("错误: " + tcpSocket->errorString());
    }
    
    // 更新连接状态
    ui->label_connect_status->setText("未连接");
    ui->label_connect_status->setStyleSheet("color: black;");
}

QString MainWindow::getIpAddress()
{
    QString ip0 = ui->textEdit_ip0->toPlainText();
    QString ip1 = ui->textEdit_ip1->toPlainText();
    QString ip2 = ui->textEdit_ip2->toPlainText();
    QString ip3 = ui->textEdit_ip3->toPlainText();
    
    // 检查IP地址格式
    bool ok0, ok1, ok2, ok3;
    int v0 = ip0.toInt(&ok0);
    int v1 = ip1.toInt(&ok1);
    int v2 = ip2.toInt(&ok2);
    int v3 = ip3.toInt(&ok3);
    
    if (!ok0 || !ok1 || !ok2 || !ok3 || 
        v0 < 0 || v0 > 255 || 
        v1 < 0 || v1 > 255 || 
        v2 < 0 || v2 > 255 || 
        v3 < 0 || v3 > 255) {
        return QString();
    }
    
    return QString("%1.%2.%3.%4").arg(v0).arg(v1).arg(v2).arg(v3);
}

int MainWindow::getPort()
{
    QString portStr = ui->textEdit_port->toPlainText();
    bool ok;
    int port = portStr.toInt(&ok);
    
    if (!ok || port <= 0 || port > 65535) {
        return -1;
    }
    
    return port;
}

QByteArray MainWindow::convertToHex(const QString &text)
{
    QByteArray result;
    
    // 移除所有空格
    QString hexText = text.simplified().remove(' ');
    
    // 确保字符串长度是偶数
    if (hexText.length() % 2 != 0) {
        hexText.prepend("0");
    }
    
    // 转换为十六进制数据
    bool ok;
    for (int i = 0; i < hexText.length(); i += 2) {
        QString byteStr = hexText.mid(i, 2);
        uint8_t byte = byteStr.toUInt(&ok, 16);
        if (ok) {
            result.append(byte);
        }
    }
    
    return result;
}

