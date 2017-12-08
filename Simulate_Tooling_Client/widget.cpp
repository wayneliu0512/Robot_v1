#include "widget.h"
#include "ui_widget.h"
#include <QNetworkInterface>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead_socket()));

    connect(this, SIGNAL(output(QString)), ui->textBrowser, SLOT(append(QString)));

    initialLineEdit();
}

Widget::~Widget()
{
    delete ui;
}

void Widget::initialLineEdit()
{
    foreach (QHostAddress address, QNetworkInterface::allAddresses()) {
        if(address.protocol() == QAbstractSocket::IPv4Protocol && address != QHostAddress(QHostAddress::LocalHost))
            ui->lineEdit_IP->setText(address.toString());
    }
}

void Widget::connected()
{
    emit output("Connected.");
    ui->lineEdit->setFocus();
}

void Widget::disconnected()
{
    emit output("Disconnected.");
}

void Widget::readyRead_socket()
{
    //Read socket
    QString readData = socket->readAll();

    emit output("Receive << " + readData);
}

void Widget::on_pushButton_Send_clicked()
{
    QString str(ui->lineEdit->text());
    sendJson(str);
}

void Widget::sendJson(const QString &_testName)
{
    QJsonObject jsonObj;
    jsonObj.insert("TestName", _testName);
    jsonObj.insert("TestStage", testStage);
    jsonObj.insert("TestResult", testResult);

    QJsonDocument jsonDoc(jsonObj);

    socket->write(jsonDoc.toJson());

    emit output("SendToSocket >> " + QString::fromUtf8(jsonDoc.toJson()));
}

void Widget::on_pushButton_Connect_clicked()
{
    QString IP = "172.16.4.73";

    if(!ui->lineEdit_IP->text().isEmpty())
        IP = ui->lineEdit_IP->text();

    emit output("Connecting to host: IP:" + IP + "    Port:1234");
    socket->connectToHost(IP, 1234);
    if(!socket->waitForConnected(1000))
    {
        emit output("Connect fail.");
        return;
    }

    socket->write(ui->comboBox_Tooling->currentText().toLatin1());
}

void Widget::on_pushButton_Disconnect_clicked()
{
    socket->disconnectFromHost();
    emit output("Disconnect from host.");
}

void Widget::on_pushButton_Clear_clicked()
{
    ui->textBrowser->clear();
}

void Widget::on_comboBox_TestStage_activated(const QString &arg1)
{
    if(arg1 == "testing")
    {
        testStage = 0;
    }else if(arg1 == "pass")
    {
        testStage = 1;
        testResult = 1;
    }else if(arg1 == "fail")
    {
        testStage = 1;
        testResult = 0;
    }else
    {
        emit output("Error: ComboBox exception.");
    }
}
