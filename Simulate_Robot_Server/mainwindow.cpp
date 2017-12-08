#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QXmlStreamReader>
#include <QTimer>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(this, SIGNAL(output(QString)), ui->textBrowser, SLOT(append(QString)));

    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), SLOT(serverError()));
    if(!server->listen(QHostAddress::Any, 54600))
    {
        emit output("Server online error.");
        close();
    }else
    {
        emit output("Server online.");
    }
}

MainWindow::~MainWindow()
{
    server->close();
    delete ui;
}

void MainWindow::newConnection()
{
    socket = server->nextPendingConnection();
    emit output("New connection IP: " + socket->peerAddress().toString());
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(socketError()));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}

void MainWindow::readyRead()
{
    QString str = socket->readAll();
    emit output("ReadyRead << " + str);

    QXmlStreamReader xml;
    xml.addData(str);
    while (!xml.atEnd())
    {
        if(xml.isStartElement())
        {
            emit output("XML:" + xml.name().toString());
            if(xml.name() == "CMD" || xml.name() == "MACHINE" || xml.name() == "UPDATEBASE" ||
               xml.name() == "UPDATESETTING")
            {
                QString ID = xml.attributes().value("ID").toString();
                ID_List.append(ID);
                QString send = ID + ",ACK";
                socket->write(send.toLatin1());
                QTimer::singleShot(2000, this, SLOT(sendDone()));
                emit output("Send >> " + send);
                break;
            }
        }
        xml.readNext();
    }
    emit output("Out of while.");
}

void MainWindow::sendDone()
{
    QString send = ID_List.first() + ",DONE";
    ID_List.pop_front();
    socket->write(send.toLatin1());
    emit output("Send >> " + send);
}

void MainWindow::serverError()
{
    emit output("Server error: " + server->errorString());
}

void MainWindow::socketError()
{
    emit output("Socket error: " + socket->errorString());
}

void MainWindow::on_pushButton_Clear_clicked()
{
    ui->textBrowser->clear();
}
