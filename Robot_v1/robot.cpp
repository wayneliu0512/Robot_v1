#include "robot.h"
#include "ui_robot.h"
#include "light.h"
#include "communication.h"
#include "task.h"
#include "base.h"
#include "mainwindow.h"
#include <QTimer>
#include <QMessageBox>
#include <QXmlStreamAttributes>
#include <QDebug>

Robot::State Robot::state = Robot::WAITING;

Robot::Robot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Robot)
{
    ui->setupUi(this);

    communication = new Communication(this, Communication::ACK, Communication::CONNECT_TO_HOST, Communication::XML);
    connect(communication, SIGNAL(online(QString)), ui->label_light, SLOT(OnlineLighting()));
    connect(communication, SIGNAL(online(QString)), ui->label_State, SLOT(setText(QString)));
    connect(communication, SIGNAL(offline()), ui->label_light, SLOT(OfflineLighting()));
    connect(communication, SIGNAL(error(QString)), ui->label_light, SLOT(ErrorLighting()));
    connect(communication, SIGNAL(error(QString)), ui->label_State, SLOT(setText(QString)));
    connect(this, SIGNAL(inAciton()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(waiting()), ui->label_light, SLOT(OnlineLighting()));

    connect(communication, SIGNAL(receiveDONE(QString)), this, SLOT(actionFinished()));
}

Robot::~Robot()
{
    delete ui;

}

void Robot::setPortIP(int _port, const QString &_IP)
{
    communication->setPortIP(_port, _IP);
}

void Robot::setBase(const QVector<Base> *_baseList)
{
    baseList = _baseList;
}

void Robot::updateBase()
{
   if(baseList->isEmpty())
   {
       QMessageBox::critical(this, "Error", "Error: Robot::updateBase()\n baseList empty.");
       qCritical() << "Error: Robot::updateBase() baseList empty.";
   }

   Task::createAction(Task::UPDATE_TOOLINGS_BASE);
   emit addTask();
}

void Robot::connectToRobot()
{
    communication->connectToHost();
}

void Robot::excuteTask(const Task &_task)
{
    if(_task.command == Task::TO_SCAN_SN)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", "10");
        attributes.append("TO", QString::number(_task.deviceNumber));

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::TO_SCAN_MAC)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", "10");
        attributes.append("TO", "13");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::TO_SCAN_FAIL)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", "13");
        attributes.append("TO", "12");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::TO_TOOLING)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", "13");
        attributes.append("TO", QString::number(_task.deviceNumber));

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::REPUT_TOOLING)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", QString::number(_task.deviceNumber));
        attributes.append("TO", QString::number(_task.deviceNumber));
        attributes.append("RELOAD", "0");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::POWER_ON)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("POWER", "1");

        communication->sendXML(_task.ID, "ROBOT", "MACHINE", attributes, QString::number(_task.deviceNumber));
    }else if(_task.command == Task::POWER_OFF)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("POWER", "0");

        communication->sendXML(_task.ID, "ROBOT", "MACHINE", attributes, QString::number(_task.deviceNumber));
    }else if(_task.command == Task::TO_PASS)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", QString::number(_task.deviceNumber));
        attributes.append("TO", "11");
        attributes.append("RELOAD", "0");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::TO_FAIL)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", QString::number(_task.deviceNumber));
        attributes.append("TO", "12");
        attributes.append("RELOAD", "0");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::UPDATE_TRAY)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("FROM", "0");

        communication->sendXML(_task.ID, "ROBOT", "CMD", attributes, "1");
    }else if(_task.command == Task::UPDATE_BASE)
    {
        updateState(INACTION);

        QXmlStreamAttributes attributes;
        attributes.append("ID", _task.ID);
        attributes.append("X", baseList->at(_task.deviceNumber-1).X);
        attributes.append("Y", baseList->at(_task.deviceNumber-1).Y);
        attributes.append("Z", baseList->at(_task.deviceNumber-1).Z);
        attributes.append("A", baseList->at(_task.deviceNumber-1).A);
        attributes.append("B", baseList->at(_task.deviceNumber-1).B);
        attributes.append("C", baseList->at(_task.deviceNumber-1).C);

        communication->sendXML(_task.ID, "ROBOT", "UPDATEBASE", attributes, QString::number(_task.deviceNumber));
    }
    else
    {
        QMessageBox::critical(this, "Error", "Robot::excuteTask()\nTask.command exception error.");
        qCritical() << "Error: Robot::excuteTask()  Task.command exception error.";
    }
}

void Robot::actionFinished()
{
    updateState(WAITING);
}

void Robot::aboutToStop()
{
    ui->label_State->setText("Stop excute task..");
}

void Robot::updateState(State _state)
{
    switch(_state)
    {
    case INACTION:
        state = INACTION;
        ui->label_State->setText("InAction");
        emit inAciton();
        break;
    case WAITING:
        state = WAITING;
        ui->label_State->setText("Waiting");
        emit waiting();
        break;
    default:
        qCritical() << "Error: Robot::updateState()  State exception.";
        break;
    }
}


