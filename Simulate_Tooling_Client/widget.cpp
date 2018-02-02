#include "widget.h"
#include "ui_widget.h"
#include <QNetworkInterface>
#include <QMessageBox>

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->groupBox->setVisible(false);

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
    ui->lineEdit_Command->setFocus();
    out.setDevice(socket);
    out.setVersion(QDataStream::Qt_5_7);
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

    if(readData == "H611NC2009;0030641A91EC" && ui->checkBox_AutoRun->isChecked()) autoRun();
}

void Widget::sendJson(const QString &_testName)
{
    QJsonObject jsonObj;
    jsonObj.insert("TestName", _testName);
    jsonObj.insert("TestStage", testStage);
    jsonObj.insert("TestResult", testResult);

    QJsonDocument jsonDoc(jsonObj);

    out << jsonDoc.toJson();

    emit output("SendToSocket >> " + QString::fromUtf8(jsonDoc.toJson()));
}

void Widget::autoRun()
{
    QTimer::singleShot(ui->spinBox->value()*1000, this, SLOT(startFakeTest()));
}

void Widget::startFakeTest()
{
    if(ui->taskList->count() == 0)
    {
        QMessageBox::information(this, "Finished", "Tasks excute completed", QMessageBox::Cancel);
    }else
    {
        QListWidgetItem *item = ui->taskList->takeItem(0);
        if(item->text() == "Pass")
        {
            testStage = 1;
            testResult = 1;
        }else
        {
            testStage = 1;
            testResult = 0;
        }
        sendJson("Final");
        delete item;
        item = nullptr;
    }
}

void Widget::on_pushButton_Send_clicked()
{
    QString str(ui->lineEdit_Command->text());
    sendJson(str);
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

void Widget::on_checkBox_AutoRun_toggled(bool checked)
{
    if(checked)
    {
        ui->groupBox->setVisible(true);
        ui->pushButton_Send->setEnabled(false);
        ui->lineEdit_Command->setEnabled(false);
        ui->comboBox_TestStage->setEnabled(false);
        ui->label_3->setEnabled(false);
    }else
    {
        ui->groupBox->setVisible(false);
        ui->pushButton_Send->setEnabled(true);
        ui->lineEdit_Command->setEnabled(true);
        ui->comboBox_TestStage->setEnabled(true);
        ui->label_3->setEnabled(true);
    }
}

void Widget::on_pushButton_AddTask_clicked()
{
    ui->taskList->addItem(ui->comboBox_Result->currentText());
}

void Widget::on_pushButton_ClearList_clicked()
{
    ui->taskList->clear();
}

