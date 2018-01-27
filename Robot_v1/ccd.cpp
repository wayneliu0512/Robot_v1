#include "ccd.h"
#include "ui_ccd.h"
#include "task.h"
#include "mainwindow.h"
#include <QDebug>
#include <QTcpSocket>
#include <communication.h>

Ccd::Ccd(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Ccd)
{
    ui->setupUi(this);

    communication = new Communication(this, Communication::ACK, Communication::CONNECT_TO_CLIENT,
                                      Communication::JSON);

    connect(communication, SIGNAL(online(QString)), ui->label_light, SLOT(OnlineLighting()));
    connect(communication, SIGNAL(offline()), ui->label_light, SLOT(OfflineLighting()));
    connect(communication, SIGNAL(error(QString)), ui->label_light, SLOT(ErrorLighting()));
    connect(this, SIGNAL(shooting()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(waiting()), ui->label_light, SLOT(OnlineLighting()));

    connect(communication, SIGNAL(receiveErrorDONE(QString)), this, SLOT(receiveError(QString)));
    connect(communication, SIGNAL(receiveDONE(QString)), this, SLOT(actionFinished()));
    connect(communication, SIGNAL(receiveSN(QString)), this, SLOT(receiveSN(QString)));
    connect(communication, SIGNAL(receiveMAC(QString)), this, SLOT(receiveMAC(QString)));
}

Ccd::~Ccd()
{
    delete ui;
}

void Ccd::setSocket(QTcpSocket *_socket)
{
    if(!communication->setSocket(_socket))
        return;
}

void Ccd::receiveSN(const QString &_SN)
{
    ui->label_SN->setText(_SN);
    emit sendSN(_SN, ToolingNumber);
}

void Ccd::receiveMAC(const QString &_MAC)
{
    ui->label_MAC->setText(_MAC);
    emit sendMAC(_MAC, ToolingNumber);
}

void Ccd::receiveError(const QString &_ID)
{
    for(int i = 0; i < MainWindow::inActionList.length(); i++)
    {
        if(MainWindow::inActionList.at(i)->ID == _ID)
        {
            Task *task = MainWindow::inActionList[i];
            if(task->nextTask)
            {
                task->nextTask->deleteNextAll();
                task->nextTask = nullptr;
            }
            task->nextTask = Task::getAction(Task::SCAN_ERROR_TO_FAIL, task->deviceNumber);
        }
    }
}

void Ccd::actionFinished()
{
    updateState(WAITING);
}

void Ccd::excuteTask(const Task &_task)
{

    if(_task.command == Task::START_SCAN)
    {
        ToolingNumber = _task.deviceNumber;
        communication->sendJSON(_task.ID, "Shot");
        updateState(SHOOTING);
    }else
    {
        qCritical() << "Error: Ccd::excuteTask()\nCase exception";
    }
}

void Ccd::updateState(State _state)
{
    switch(_state)
    {
    case Ccd::CONNECTED:
        state = CONNECTED;
        emit connected();
        break;
    case Ccd::SHOOTING:
        state = SHOOTING;
        emit shooting();
        break;
    case Ccd::WAITING:
        state = WAITING;
        emit waiting();
        break;
    default:
        qCritical() << "Error: Ccd::updateState() exception error.";
        break;
    }
}
