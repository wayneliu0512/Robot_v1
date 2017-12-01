#ifndef TOOLING_H
#define TOOLING_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QSqlDatabase>

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
    void setToolingSN(const QString _toolingSN);
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
    void receiveMessage(const QString &_testName, const int &_testStage, const int &_testResult);
    void clockUpdate();
private:
    Ui::Tooling *ui;

    void initializeClock();
    void initialTestList();

    bool updateState(State _nextState);

    void sendToClient_SN_MAC();
//    void updateTestTime();
    void receive_AllTestFinished(const int &_testResult);
    void receive_TestStart(const QString &_testName);
    void receive_TestFinished(const QString &_testName, const int &_testResult);

    void getMoBySN(const QString &_SN);
    void insertDb(const QString &_testName);
    void updateDb(const QString &_testName, const QString &_result, int _testTime);

//    QSqlDatabase db;
    State state = CONNECTED;
    TestTime testTime = ZERO_TIME;
    QString toolingSN;// 機台序號
    int toolingNumber;// 機台位置編號
    QTimer clockTimer;
    QTime clockTime, testStartTime;
    QString SN, MAC, MO, PN;
    QString logPath;
    QStringList testItemList;
    QMessageBox messageBox;
};

#endif // TOOLING_H
