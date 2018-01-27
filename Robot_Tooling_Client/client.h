#ifndef SOCKETTEST_H
#define SOCKETTEST_H

#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QtCore>
#include <AutoItX/AutoItX3_DLL.h>
#include <QSerialPort>

class Client : public QObject
{
    Q_OBJECT
public:
    explicit Client(QObject *parent = 0);

    ~Client();

public slots:

    void connected();
    void disconnected();
    void readyRead_socket();
    void readyRead_serial();

    //Check whether the boot is successful
    void timeoutForBoot();
    void reConnectTimeout();

private:

    QSerialPort *serial;
    QTcpSocket *socket;
    QDataStream out;

    QString SN, MAC1;
    QString ip, com, toolingNum;
    int port;

    QTimer *timer;
    QTimer *timerForReConnect;
    int timerSecond_checkOnline,  timerCounter = 0;

    QString testName;

    bool reTestSignal = false;

    //讀取Ini設定檔內容
    void readIniFile();

    //Open Serial port to connect
    void openSerialPort();

    //Start connect socket to host
    void connectToHost();

//    testStage: 0 = Start test
//               1 = Test finished
//    testResult: 0 = fail
//                1 = pass
    void sendJson(const QString &_testName, const int &_testStage, const int &_testResult);

    bool controlWAutoMESProgram_PASS();
    void controlWAutoMESProgram_FAIL();
    void controlWAutoMESProgram_TimeoutFAIL();

    //    void controlWBarcodeProgram(const QString &SN_string , const QString &MAC1_string);

};

#endif // SOCKETTEST_H
