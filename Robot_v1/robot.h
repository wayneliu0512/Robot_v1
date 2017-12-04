#ifndef ROBOT_H
#define ROBOT_H

#include <QWidget>

class Communication;
class Task;
class Base;

namespace Ui {
class Robot;
}

class Robot : public QWidget
{
    Q_OBJECT

public:
    enum State{ INACTION, WAITING };//手臂狀態
    explicit Robot(QWidget *parent = 0);
    ~Robot();

    void setPortIP(int _port, const QString &_IP);
    void setBase(const QVector<Base> *_baseList);//設置各機台Base 列表
    void connectToRobot();
    void updateBase(); //派發更新Base任務

    static State state;

    Communication *communication;
signals:
//    更新手臂狀態輸出
    void inAciton();
    void waiting();

//    新增任務輸出
    void addTask();
public slots:
    void excuteTask(const Task &_task);// 接收並執行任務
    void aboutToStop();
private slots:
    void actionFinished();
private:
    Ui::Robot *ui;

    void updateState(State _state);//更新手臂狀態

    QTimer *timer;
    const QVector<Base> *baseList;
};

#endif // ROBOT_H
