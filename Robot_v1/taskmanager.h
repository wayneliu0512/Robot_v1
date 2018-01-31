#ifndef TASKMANAGER_H
#define TASKMANAGER_H
#include <QThread>
/* 派發任務系統：
   當系統開始運作, 新的線程while迴圈會開始作動, 持續檢查有無任務可以執行, 並且派發任務
*/
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
    void excuteRobotTask(const Task &_task);    //派發Robot任務
    void excuteToolingTask(const Task &_task);  //派發Tooling任務
    void excuteCcdTask(const Task &_task);      //派發Ccd任務
public slots:
    void systemActionFinished();                //當一個動作完成時, 更新系統狀態
};

#endif // TASKMANAGER_H
