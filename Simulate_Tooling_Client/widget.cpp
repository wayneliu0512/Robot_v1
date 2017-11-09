#include "widget.h"
#include "ui_widget.h"

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

    emit output("Connecting to host: IP:172.16.4.73    Port:1234");

    socket->connectToHost("172.16.4.73", 1234);//"192.168.0.103" , "172.16.4.101", "10.211.55.3"
    if(!socket->waitForConnected(1000))
    {
        emit output("Connect fail.");
    }
}

Widget::~Widget()
{
    delete ui;
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
    socket->write(str.toLatin1());
    emit output("Send >> " + str);
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
    }
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
