#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QThread>

class QTimer;
class Task;

class TaskManager : public QThread
{
    Q_OBJECT
public:
    explicit TaskManager(QObject *parent = 0);
    ~TaskManager();
protected:
    void run();
signals:
    void excuteRobotTask(const Task &_task);
    void excuteToolingTask(const Task &_task);
    void excuteCcdTask(const Task &_task);
public slots:
    void systemActionFinished();
};

#endif // TASKMANAGER_H
