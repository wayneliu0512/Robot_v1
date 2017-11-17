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
    explicit Client(const QString &_SN, const QString &_MAC, QObject *parent = 0);

    ~Client();

public slots:

    void readyRead_serial();

    //Check whether the boot is successful
    void timeoutForBoot();

private:

    QSerialPort *serial;

    QString SN, MAC1;
    QString com;
    QString testName;

    QTimer *timer;
    int timerSecond_checkOnline,  timerCounter = 0;

    //讀取Ini設定檔內容
    void readIniFile();

    void sendJson(const QString &_testName, const int &_testStage, const int &_testResult);

    //Open Serial port to connect
    void openSerialPort();
};

#endif // SOCKETTEST_H
