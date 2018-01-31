#include "task.h"
#include <QDebug>
#include <QTime>
#include <QThread>
#include "mainwindow.h"

Task::Task(Task::Command _command, Task::Device _device, int _deviceNumber) :
    command(_command), device(_device), deviceNumber(_deviceNumber)
{
    QThread::msleep(10);
    ID = QString::number(QDateTime::currentMSecsSinceEpoch());
    nextTask = nullptr;
}

Task::~Task()
{
}

void Task::deleteNextAll()
{
    if(nextTask)
    {
        nextTask->deleteNextAll();
    }
//    qDebug() << "Task delete: " << commandToString();
    delete this;
}

Task* Task::next(Task *_nextTask)
{
    nextTask = _nextTask;
    return nextTask;
}

Task* Task::takeNextTask()
{
    Task *outputNextTask = nextTask;
    nextTask = nullptr;

    return outputNextTask;
}

QString Task::commandToString()
{
    switch(command)
    {
    case TO_SCAN_SN:
        return "ToScanSN";
    case TO_SCAN_MAC:
        return "ToScanMAC";
    case TO_SCAN_FAIL:
        return "ToScanFail";
    case START_SCAN:
        return "StartScan";
    case TO_TOOLING:
        return "ToTooling";
    case REPUT_TOOLING:
        return "RePutTooling";
    case TO_PASS:
        return "ToPass";
    case TO_FAIL:
        return "ToFail";
    case POWER_ON:
        return "PowerOn";
    case POWER_OFF:
        return "PowerOff";
    case UPDATE_TRAY:
        return "UpdateTray";
    case UPDATE_BASE:
        return "UpdateBase";
    case UPDATE_SETTING:
        return "UpdateSetting";
    default:
        qCritical() << "Error: Task::commandToString()";
        return "NULL";
    }
}

QString Task::deviceToString()
{
    switch(device)
    {
    case ROBOT:
        return "Robot";
    case TOOLING:
        return "Tooling";
    case CCD:
        return "CCD";
    default:
        qCritical() << "Error: Task::deviceToString()";
        return "NULL";
    }
}

void Task::createAction(Action _action, int _deviceNumber)
{
    if(_action == GET_DUT_TO_TEST)
    {
        Task *taskToScanSN = new Task(Task::TO_SCAN_SN, Task::ROBOT, _deviceNumber);
        Task *taskStartScanSN = new Task(Task::START_SCAN, Task::CCD, _deviceNumber);
        Task *taskToScanMAC = new Task(Task::TO_SCAN_MAC, Task::ROBOT, _deviceNumber);
        Task *taskStartScanMAC = new Task(Task::START_SCAN, Task::CCD, _deviceNumber);
        Task *taskToTooling = new Task(Task::TO_TOOLING, Task::ROBOT, _deviceNumber);
        Task *taskPowerOn = new Task(Task::POWER_ON, Task::TOOLING, _deviceNumber);
        Task *taskUpdateTray = new Task(Task::UPDATE_TRAY, Task::ROBOT);

        taskToScanSN->next(taskStartScanSN)
                ->next(taskToScanMAC)
                ->next(taskStartScanMAC)
                ->next(taskToTooling)
                ->next(taskPowerOn)
                ->next(taskUpdateTray);

        MainWindow::waitingList.append(taskToScanSN);

    }else if(_action == TEST_FINISHED_TO_PASS)
    {
        Task *taskPowerOff = new Task(Task::POWER_OFF, Task::TOOLING, _deviceNumber);
        Task *taskToPass = new Task(Task::TO_PASS, Task::ROBOT, _deviceNumber);
        Task *taskUpdateTray = new Task(Task::UPDATE_TRAY, Task::ROBOT);

        taskPowerOff->next(taskToPass)
                ->next(taskUpdateTray);

        MainWindow::waitingList.append(taskPowerOff);

    }else if(_action == TEST_FINISHED_TO_FAIL)
    {
        Task *taskPowerOff = new Task(Task::POWER_OFF, Task::TOOLING, _deviceNumber);
        Task *taskToFail = new Task(Task::TO_FAIL, Task::ROBOT, _deviceNumber);
        Task *taskUpdateTray = new Task(Task::UPDATE_TRAY, Task::ROBOT);

        taskPowerOff->next(taskToFail)
                ->next(taskUpdateTray);

        MainWindow::waitingList.append(taskPowerOff);

    }else if(_action == TEST_FAIL_TO_RETEST)
    {
        Task *taskPowerOff = new Task(Task::POWER_OFF, Task::TOOLING, _deviceNumber);
        Task *taskRePutTooling = new Task(Task::REPUT_TOOLING, Task::ROBOT, _deviceNumber);
        Task *taskPowerOn = new Task(Task::POWER_ON, Task::TOOLING, _deviceNumber);

        taskPowerOff->next(taskRePutTooling)
                ->next(taskPowerOn);

        MainWindow::waitingList.append(taskPowerOff);

    }else if(_action == UPDATE_ALLSET){

        Task *taskTooling1 = new Task(Task::UPDATE_BASE, Task::ROBOT, 1);

        if(MainWindow::toolingQuantity == 2)
        {
            Task *taskTooling2 = new Task(Task::UPDATE_BASE, Task::ROBOT, 2);

            taskTooling1->next(taskTooling2);

        }else if(MainWindow::toolingQuantity > 2)
        {
            Task *tempTask = taskTooling1->next(new Task(Task::UPDATE_BASE, Task::ROBOT, 2));
            for(int i = 0; i < MainWindow::toolingQuantity - 2; i++)
            {
                tempTask = tempTask->next(new Task(Task::UPDATE_BASE, Task::ROBOT, i + 3));
            }
        }
        MainWindow::waitingList.prepend(taskTooling1);
    }else
    {
        qCritical() << "Error: Task::createAction()  Case exception.";
    }
}

Task *Task::getAction(Action _action, int _deviceNumber)
{
    if(_action == SCAN_ERROR_TO_FAIL)
    {
        Task *taskToFail = new Task(Task::TO_SCAN_FAIL, Task::ROBOT, _deviceNumber);
        Task *taskUpdateTray1 = new Task(Task::UPDATE_TRAY, Task::ROBOT);
        Task *taskUpdateTray2 = new Task(Task::UPDATE_TRAY, Task::ROBOT);
        Task *taskToScanSN = new Task(Task::TO_SCAN_SN, Task::ROBOT, _deviceNumber);
        Task *taskStartScanSN = new Task(Task::START_SCAN, Task::CCD, _deviceNumber);
        Task *taskToScanMAC = new Task(Task::TO_SCAN_MAC, Task::ROBOT, _deviceNumber);
        Task *taskStartScanMAC = new Task(Task::START_SCAN, Task::CCD, _deviceNumber);
        Task *taskToTooling = new Task(Task::TO_TOOLING, Task::ROBOT, _deviceNumber);
        Task *taskPowerOn = new Task(Task::POWER_ON, Task::TOOLING, _deviceNumber);

        taskToFail->next(taskUpdateTray1)
                ->next(taskToScanSN)
                ->next(taskStartScanSN)
                ->next(taskToScanMAC)
                ->next(taskStartScanMAC)
                ->next(taskToTooling)
                ->next(taskPowerOn)
                ->next(taskUpdateTray2);

        return taskToFail;
    }else
    {
        qCritical() << "Error: Task::getAction()  Case exception";
        return nullptr;
    }
}
