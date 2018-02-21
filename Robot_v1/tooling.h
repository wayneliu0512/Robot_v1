#ifndef TOOLING_H
#define TOOLING_H

#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QDebug>

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
//    對於機箱, 只會在這幾種狀態中, 依循規則的切換, 成為固定的流程, 利用這個規則可以確保流程是正確的
    enum State{ CONNECTED, IDLE, RE_TEST, TESTING, RE_TESTING, TEST_FINISHED_PASS, TEST_FINISHED_FAIL, ERROR };
//    機箱測試的次數, 最多測兩次
    enum TestTime{ ZERO_TIME, FIRST_TIME, SECOND_TIME };

    explicit Tooling(QWidget *parent = 0);
    ~Tooling();

    void setToolingNumber(int _number);                                   //設置機箱位置號碼
    void setToolingSN(const QString _toolingSN){ toolingSN = _toolingSN; }//設置機箱序號
    void setSocket(QTcpSocket *_socket);                                  //設置socket
    void setLogPath(const QString &_path){ logPath = _path; }             //設置Log存放路徑

    Communication *communication;

signals:
    void addTask();// 新增任務

    void excuteTaskByRobot(const Task &_task);// 傳遞給手臂執行任務
//    狀態更新的輸出
    void idle();        //機台閒置
    void testing();     //機台測試中
    void testFinished();//機台測試結束
    void reTest();      //機台重新測試
    void reTesting();   //機台重新測試中

    void receiveACK(const QString &id);
    void receiveDONE(const QString &id);
    void waiting();

    void error(const QString &_str);
public slots:
    void excuteTask(const Task &_task);//執行任務

    void receiveSN(const QString &_SN, const int &_toolingNumber);  //Ccd傳來的SN
    void receiveMAC(const QString &_MAC, const int &_toolingNumber);//Ccd傳來的MAC
    void errorManage(const QString &_str);
private slots:
    //接收來自機箱的訊息
    void receiveMessage(const QString &_testName, const int &_testStage, const int &_testResult);
    void clockUpdate();//測試計時用

    void on_disconnectButton_clicked();

private:
    Ui::Tooling *ui;
//    初始化計時器跟測試清單
    void initializeClock();
    void initialTestList();

//    用於把關機台狀態流程正確, 如果流程有問題, return false
    bool updateState(State _nextState);

//    將SN, MAC傳送給機台
    void sendToClient_SN_MAC();

//    是由receiveMessage()分支出來的三種訊息, 1.測試開始 2.測試結束 3.全部測試結束
    void receive_AllTestFinished(const int &_testResult);
    void receive_TestStart(const QString &_testName);
    void receive_TestFinished(const QString &_testName, const int &_testResult);

//    從 Web Service 取得 Mo
    void getMoBySN(const QString &_SN);

//    上傳Db, 測試開始時 insert, 測試結束時 update 相對應的測試項目
    void insertDb(const QString &_testName);
    void updateDb(const QString &_testName, const QString &_result, int _testTime);
    void updateDbVersion();

    void toolDisconnect();


    State state = CONNECTED;        //機台狀態
    TestTime testTime = ZERO_TIME;  //機台測試次數
    QString toolingSN;              //機台序號
    int toolingNumber;              //機台位置編號
    QString dsn;                    //DB 使用的Dsn碼
    int dbVersion = 0;              //DB 使用的Version
    QTimer clockTimer;
    QTime clockTime, testStartTime;
    QString SN, MAC, MO, PN;
    QString logPath;
    QStringList testItemList;
    QMessageBox messageBox;
    bool disconnectFlag=false;
};

#endif // TOOLING_H
