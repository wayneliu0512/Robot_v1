#include "taskmanager.h"
#include "mainwindow.h"
#include "task.h"
#include "robot.h"
#include <QDebug>
#include <QTimer>

TaskManager::TaskManager(QObject *parent) : QThread(parent)
{
    qRegisterMetaType<Task>("Task");
}

TaskManager::~TaskManager()
{

}

void TaskManager::run()
{
    while(1)
    {
        msleep(200);

        if(MainWindow::systemState == MainWindow::STOP)
        {
            qDebug() << "TaskManager Stop";
            break;
        }

        if(MainWindow::waitingList.isEmpty())
            continue;

        if( (Robot::state == Robot::INACTION) || (MainWindow::systemAction == MainWindow::INACTION) )
            continue;

        MainWindow::systemAction = MainWindow::INACTION;

        qDebug() << "excute Task";

        Task *task = MainWindow::waitingList.first();

        switch (task->device) {
        case Task::ROBOT:
            emit excuteRobotTask(*task);
            break;
        case Task::TOOLING:
            emit excuteToolingTask(*task);
            break;
        case Task::CCD:
            emit excuteCcdTask(*task);
            break;
        default:
            qDebug() << "Error: TaskManager::startRunningTasks / Device exception.";
            break;
        }
    }
}

void TaskManager::systemActionFinished()
{
    MainWindow::systemAction = MainWindow::WAITING;
}
