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
    emit output("Connected!");
    QString str = "ccd";
    socket->write(str.toLatin1());
    emit output("Send >> " + str);
}

void Widget::disconnected()
{
    emit output("Disconnected!");
}

void Widget::readyRead_socket()
{
    //Read socket
    QByteArray readData = socket->readAll();

    //Tranfer socket to Json Format , get ID and CMD
    QJsonDocument doc = QJsonDocument::fromJson(readData);
    QJsonValue cmd_value = doc.object().value("CMD");
    ID_value = doc.object().value("ID");

    emit output("Read << CmdValue: " + cmd_value.toString());
    //If Command is Shot , then take a shot
    if(cmd_value.toString() == "Shot")
    {
        //Create Json Format data to do ACK respond
        QJsonObject ccd_Obj_ACK;
        ccd_Obj_ACK.insert("ID", ID_value);
        ccd_Obj_ACK.insert("Type", "ACK");
        QJsonDocument doc(ccd_Obj_ACK);
        QByteArray sendData = doc.toJson();

        //Do ACK respond
        socket->write(sendData);

        emit output("Send >> " + sendData);

        //Take a shot here...

        //Wait for 2 Second to simulate Taking a shot
        //After taking a shot , do DONE respond
        QTimer::singleShot(500, this, &Widget::DONE_Respond);
    }
}

//Create Json Format data to do DONE respond
void Widget::DONE_Respond()
{
    //Create Json Format data to do DONE respond
    QJsonObject ccd_Obj_DONE;
    ccd_Obj_DONE.insert("ID", ID_value);
    ccd_Obj_DONE.insert("Type", "DONE");
    ccd_Obj_DONE.insert("SN", ui->lineEdit_SN->text());
    ccd_Obj_DONE.insert("MAC", ui->lineEdit_MAC->text());
    if(ui->checkBox->isChecked())
        ccd_Obj_DONE.insert("Error", "Error test.");
    QJsonDocument doc(ccd_Obj_DONE);
    QByteArray sendData = doc.toJson();

    //Do ACK respond
    socket->write(sendData);
    emit output("Send >> " + sendData);
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
    emit output("disconnect from host.");
}

void Widget::on_pushButton_Clear_clicked()
{
    ui->textBrowser->clear();
}
