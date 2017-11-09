#ifndef TOOLING_H
#define TOOLING_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QMessageBox>

class Communication;
class QTcpSocket;
class Task;

namespace Ui {
class Tooling;
}

class Tooling : public QWidget
{
    Q_OBJECT

public:
    enum State{ CONNECTED, IDLE, RE_TEST, TESTING, RE_TESTING, TEST_FINISHED_PASS, TEST_FINISHED_FAIL, ERROR};
//    enum Action{ TO_FAIL, TO_PASS, GET_SINK};
    enum TestTime{ ZERO_TIME, FIRST_TIME, SECOND_TIME };

    explicit Tooling(QWidget *parent = 0);
    ~Tooling();

    void setToolingNumber(int _number);
    void setSocket(QTcpSocket *_socket);
    void setLogPath(const QString &_path);

    Communication *communication;
signals:
    void addTask();

    void excuteTaskByRobot(const Task &_task);

    void idle();
    void testing();
    void testFinished();
    void reTest();
    void reTesting();
    void error(const QString &_str);
public slots:
    void excuteTask(const Task &_task);

    void receiveSN(const QString &_SN, const int &_toolingNumber);
    void receiveMAC(const QString &_MAC, const int &_toolingNumber);
    void errorManage(const QString &_str);
private slots:
    void receiveMessage(const QString &_message);
    void clockUpdate();
private:
    Ui::Tooling *ui;

    void initializeClock();
    void initialTestList();
    void testListUpdate(const QString &_message);
    bool updateState(State _nextState);
    void sendToClient_SN_MAC();
//    void updateTestTime();

    State state = CONNECTED;
    TestTime testTime = ZERO_TIME;
    int toolingNumber;
    QTimer clockTimer;
    QTime clockTime;
    QString SN, MAC;
    QString logPath;
    QStringList testItemList;
    QMessageBox messageBox;
};

#endif // TOOLING_H
