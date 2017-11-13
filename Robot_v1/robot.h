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
    enum State{ INACTION, WAITING };
    explicit Robot(QWidget *parent = 0);
    ~Robot();

    void setPortIP(int _port, const QString &_IP);
    void setBase(const QVector<Base> *_baseList);
    void connectToRobot();
    void updateBase();

    static State state;

    Communication *communication;
signals:
    void inAciton();
    void waiting();
    void addTask();

public slots:
    void excuteTask(const Task &_task);
    void aboutToStop();
private slots:
    void actionFinished();
private:
    Ui::Robot *ui;

    void updateState(State _state);

    QTimer *timer;
    const QVector<Base> *baseList;
};

#endif // ROBOT_H
