#include "tooling.h"
#include "ui_tooling.h"
#include "communication.h"
#include "task.h"
#include <QTcpSocket>
#include "mainwindow.h"
#include <QTimer>
#include <QTime>
#include <QFile>

Tooling::Tooling(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tooling)
{
    ui->setupUi(this);

    communication = new Communication(this, Communication::NO_ACK, Communication::CONNECT_TO_CLIENT);

    connect(communication, SIGNAL(online(QString)), ui->label_light, SLOT(OnlineLighting()));
    connect(communication, SIGNAL(offline()), ui->label_light, SLOT(OfflineLighting()));
    connect(communication, SIGNAL(error(QString)), this, SLOT(errorManage(QString)));
    connect(communication, SIGNAL(error(QString)), this, SLOT(errorManage(QString)));
    connect(this, SIGNAL(idle()), ui->label_light, SLOT(OnlineLighting()));
    connect(this, SIGNAL(testing()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(reTesting()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(testFinished()), ui->label_light, SLOT(OnlineLighting()));
    connect(this, SIGNAL(error(QString)), ui->label_State, SLOT(setText(QString)));
    connect(this, SIGNAL(error(QString)), ui->label_light, SLOT(ErrorLighting()));

    initializeClock();
    connect(&clockTimer, SIGNAL(timeout()), this, SLOT(clockUpdate()));

    connect(communication, SIGNAL(receiveMessage(QString)), this, SLOT(receiveMessage(QString)));

    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setWindowTitle("Error");
}

Tooling::~Tooling()
{
    delete ui;

}

void Tooling::initializeClock()
{
    clockTimer.stop();

    clockTime.setHMS(0,0,0);

    ui->clock->display(clockTime.toString("mm:ss"));
}

void Tooling::clockUpdate()
{
    clockTime = clockTime.addSecs(1);

    QString clockText = clockTime.toString("mm:ss");

    if ((clockTime.second() % 2) == 0)
        clockText[2] = ' ';

    ui->clock->display(clockText);
}

void Tooling::setToolingNumber(int _number)
{
    toolingNumber = _number;
    ui->groupBox->setTitle("Tooling" + QString::number(_number));
}

void Tooling::setSocket(QTcpSocket *_socket)
{
    if(!communication->setSocket(_socket))
        return;

    updateState(IDLE);
}

void Tooling::setLogPath(const QString &_path)
{
    logPath = _path;
}

void Tooling::receiveMessage(const QString &_message)
{
    qDebug() << "Tooling" << QString::number(toolingNumber) << " receive message: " << _message;
    testListUpdate(_message);
    if(_message == "test pass")
    {
        updateState(TEST_FINISHED_PASS);
    }else if(_message == "test fail")
    {
        updateState(TEST_FINISHED_FAIL);
    }else if(_message == "AutoMES ERROR")
    {
        updateState(TEST_FINISHED_FAIL);
    }else
    {
        qDebug() << "Tooling"<< QString::number(toolingNumber) << " exception message: " << _message;
    }
}

void Tooling::receiveSN(const QString &_SN, const int &_toolingNumber)
{
    if(_toolingNumber != toolingNumber)
        return;
    SN = _SN;
    ui->label_SN->setText(SN);
}

void Tooling::receiveMAC(const QString &_MAC, const int &_toolingNumber)
{
    if(_toolingNumber != toolingNumber)
        return;
    MAC = _MAC;
    ui->label_MAC->setText(MAC);
}

void Tooling::sendToClient_SN_MAC()
{
    QString sendData = SN + ";" + MAC;
    communication->send(sendData, toolingNumber);
}

void Tooling::excuteTask(const Task &_task)
{
    if(_task.deviceNumber != toolingNumber)
        return;

    if(_task.command == Task::POWER_ON)
    {
        if(testTime == ZERO_TIME)
        {
            testTime = FIRST_TIME;
            if(!updateState(TESTING))
                return;
        }
        else if(testTime == FIRST_TIME)
        {
            testTime = SECOND_TIME;
            if(!updateState(RE_TESTING))
                return;
        }
        else
        {
            qCritical() << "Error: Tooling::excuteTask()/Task::POWER_ON  Case exception error.";
            return;
        }
        clockTimer.start(1000);
        sendToClient_SN_MAC();
        emit excuteTaskByRobot(_task);
    }
    else if(_task.command == Task::POWER_OFF)
    {
        if(state == TEST_FINISHED_PASS)
        {
            if(!updateState(IDLE))
                return;
        }
        else if(state == TEST_FINISHED_FAIL)
        {
            if(testTime == FIRST_TIME)
            {
                if(!updateState(RE_TEST))
                    return;
            }else if(testTime == SECOND_TIME)
            {
                if(!updateState(IDLE))
                    return;
            }else
            {
                qCritical() << "Error: Tooling::excuteTask()/Task::POWER_OFF/TEST_FINISHED  Case exception error.";
                return;
            }
        }
        else
        {
            qCritical() << "Error: Tooling::excuteTask()/Task::POWER_OFF  Case exception error.";
            return;
        }
        emit excuteTaskByRobot(_task);
    }

}

//有機會可以用狀態機作
bool Tooling::updateState(State _nextState)
{
    switch (_nextState) {
    case CONNECTED:

        break;
    case IDLE:
        if(state != CONNECTED && state != TEST_FINISHED_PASS && state != TEST_FINISHED_FAIL)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict0");
            return false;
        }

        testTime = ZERO_TIME;
        emit idle();
        ui->label_State->setText("Idle");
        Task::createAction(Task::GET_DUT_TO_TEST, toolingNumber);
        emit addTask();

        break;
    case TESTING:
        if(state != IDLE)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict1");
            return false;
        }
        emit testing();
        ui->label_State->setText("Testing");
        break;
    case TEST_FINISHED_PASS:
        if(state != TESTING && state != RE_TESTING)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict2");
            return false;
        }
        emit testFinished();
        initialTestList();
        initializeClock();
        ui->label_State->setText("TestFinished");
        Task::createAction(Task::TEST_FINISHED_TO_PASS, toolingNumber);
        emit addTask();

        break;
    case TEST_FINISHED_FAIL:
        if(state != TESTING && state != RE_TESTING)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict3");
            return false;
        }
        emit testFinished();
        initialTestList();
        initializeClock();
        ui->label_State->setText("TestFinished");
        if(testTime == FIRST_TIME)
        {
            Task::createAction(Task::TEST_FAIL_TO_RETEST, toolingNumber);
        }else if(testTime == SECOND_TIME)
        {
            Task::createAction(Task::TEST_FINISHED_TO_FAIL, toolingNumber);
        }
        else
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict4");
            return false;
        }
        emit addTask();

        break;
    case RE_TEST:
        if(state != TEST_FINISHED_FAIL)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict5");
            return false;
        }
        emit reTest();
        ui->label_State->setText("ReTest");
        break;
    case RE_TESTING:
        if(state != RE_TEST)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict6");
            return false;
        }
        emit reTesting();
        ui->label_State->setText("ReTesting");
        break;
    default:
        errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict7");
        return false;
    }

    state = _nextState;
    return true;
}

void Tooling::testListUpdate(const QString &_message)
{
    testItemList.append(_message);

    ui->listWidget->insertItem(0, _message);
}

void Tooling::initialTestList()
{
    QFile logFile(logPath + "\\" + SN + ".log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&logFile);
    out << "Tooling Number : " << QString::number(toolingNumber) << endl
        << "MAC Number     : " << MAC << endl
        << "Test time spend: " << clockTime.toString("mm:ss") << " (mins/secs)"<<endl
        << "------------------------------"<< endl
        << "TestItem:" << endl;

    for(int i = 0; i < testItemList.length(); i++)
        out << testItemList.at(i) << endl;

    testItemList.clear();
    ui->listWidget->clear();
}

void Tooling::errorManage(const QString &_str)
{
    messageBox.setText(_str);
    messageBox.show();
    qCritical() << _str;
    state = ERROR;
    emit error(_str);
}
