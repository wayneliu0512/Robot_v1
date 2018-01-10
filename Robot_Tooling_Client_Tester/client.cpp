#include "client.h"
#include <QCoreApplication>

QT_USE_NAMESPACE

Client::Client(const QString &_SN, const QString &_MAC, QObject *parent) :
    SN(_SN), MAC1(_MAC), QObject(parent)
{
    readIniFile();

    //initialize Serial port
    serial = new QSerialPort;
    connect(serial, SIGNAL(readyRead()), this, SLOT(readyRead_serial()));
    serial->setPortName(com);
    serial->setBaudRate(QSerialPort::Baud57600);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);

    //initailize Timer
    timer = new QTimer;
    connect(timer, SIGNAL(timeout()), this, SLOT(timeoutForBoot()));

    openSerialPort();

}

Client::~Client()
{
    delete serial;
    delete timer;
}

void Client::readIniFile()
{
    QSettings setting("Setting.ini",QSettings::IniFormat);
    com = setting.value("Option/COMPortName").toString();
    timerSecond_checkOnline = setting.value("Option/Timer_check(sec)").toInt();
}

void Client::openSerialPort()
{
    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug() << "SerialPort can't open.";

        exit(1);
       // QCoreApplication::exit(1);
    }else
    {
        qDebug() << "SerialPort is open.";

        timerCounter = 0;
        timer->start(1000);
        qDebug() << "Timer Start";
    }
}

void Client::readyRead_serial()
{
    static QByteArray dataBuffer;
    dataBuffer.append(serial->readAll());
    serial->flush();
    qDebug() << "dataBuffer << " + dataBuffer;

    QByteArray newStr;
    foreach (char in, dataBuffer)
    {
        if(in != '\0') newStr.append(in);
    }

//    Example:
//    1.單純接收開始測試項目:     @SATA;
//    2.接收測試完成的項目:      @SATA/P;
//    3.接收全部測試完成的結果:   @PASSF;
    QRegularExpression reg("[@]{1}(?<text>\\w*|\\w*[/][FP])[;]{1}");
    QRegularExpressionMatchIterator iter = reg.globalMatch(QString(newStr));
    while (iter.hasNext()) {
        QRegularExpressionMatch match = iter.next();
        QString captureStr = match.captured("text");
        if(captureStr.contains('/'))
        {
            qDebug() << "dataConfirm << " + captureStr;

            testName = captureStr.split('/').at(0);
            QString result = captureStr.split('/').at(1);

            if(result == "P")
            {
                timerCounter = 0;
                sendJson(testName, 1, 1);
            }else
            {
                timerCounter = 0;
                sendJson(testName, 1, 0);
            }

        }else
        {
            qDebug() << "dataConfirm << " + captureStr;
//            接收全部測試完成的結果
            if(captureStr == "PASSF")
            {
                timer->stop();

                testName = "Final";
                sendJson(testName, 1, 1);
            }else if(captureStr == "FAILF")
            {
                timer->stop();

                testName = "Final";
                sendJson(testName, 1, 0);
            }
//            接收開始測試項目
            else
            {
                timerCounter = 0;

                testName = captureStr;
                sendJson(testName, 0, 0);
            }
        }
        dataBuffer.clear();
    }
    //SN request 跟Justin程式做溝通
    if(dataBuffer.contains(0xBC))
    {
        serial->write("B");
        qDebug() << "SendToDOS >> B";
        dataBuffer.clear();
    }
    //SN request 跟Justin程式做溝通
    else if(dataBuffer.contains(0x07))
    {
        QString sendData = SN + "1" + MAC1 + "                          "; // 26 spaces for Jastin.
        serial->write(sendData.toLocal8Bit().data());
        qDebug() << "SendToDOS >> " + sendData;
        dataBuffer.clear();
    }
}

void Client::sendJson(const QString &_testName, const int &_testStage, const int &_testResult)
{
    QJsonObject jsonObj;
    jsonObj.insert("TestName", _testName);
    jsonObj.insert("TestStage", _testStage);
    jsonObj.insert("TestResult", _testResult);

    QJsonDocument jsonDoc(jsonObj);

//    socket->write(jsonDoc.toJson());
    qDebug() << "SendToSocket >> " + QString::fromUtf8(jsonDoc.toJson());
}

void Client::timeoutForBoot()
{
    timerCounter++;
    if(timerCounter % 5 == 0)
        qDebug() << QString::number(timerCounter) + "Second...";

    if(timerCounter <= timerSecond_checkOnline)
        return;

    qDebug() << "Timeout fail";

    exit(1);

    timer->stop();
    timerCounter = 0;
}
