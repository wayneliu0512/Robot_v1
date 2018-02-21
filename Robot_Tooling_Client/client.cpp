#include "client.h"
#include <QRegularExpressionMatch>

Client::Client(QObject *parent) :
    QObject(parent)
{
    readIniFile();

    //initialize TCP socket
    socket = new QTcpSocket;
    connect(socket, SIGNAL(connected()), this, SLOT(connected()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead_socket()));

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

    timerForReConnect = new QTimer;
    connect(timerForReConnect, SIGNAL(timeout()), this, SLOT(reConnectTimeout()));

    //Open Serial port to connect DOS
    openSerialPort();

    //Start connecting to host
    connectToHost();
}

Client::~Client()
{
    delete socket;
    delete serial;
    delete timer;
    delete timerForReConnect;
}

void Client::readIniFile()
{
    QSettings setting("Setting.ini",QSettings::IniFormat);
    ip = setting.value("Option/IP").toString();
    port = setting.value("Option/Port").toInt();
    com = setting.value("Option/COMPortName").toString();
    toolingNum = setting.value("Option/Tooling").toString();
    //超過這個秒數, 則認定此測項卡住FAIL
    timerSecond_checkOnline = setting.value("Option/Timer_check(sec)").toInt();
}

void Client::openSerialPort()
{
    if(!serial->open(QIODevice::ReadWrite))
    {
        qDebug() << "SerialPort can't open.";
        return QCoreApplication::exit(1);
    }else
    {
        qDebug() << "SerialPort is open.";
    }
}

void Client::connectToHost()
{
    socket->connectToHost(ip, port);//"192.168.0.103" , "172.16.4.101", "10.211.55.3"
    if (socket->waitForConnected(1000))
    {
        qDebug("Socket connected to host.");
    }else
    {
        qDebug("Socket connect fail.");
        timerForReConnect->start(5000);
    }
}

void Client::connected()
{
    qDebug() << "Connected!(Socket)";
    socket->write(toolingNum.toLatin1());
    out.setDevice(socket);
    out.setVersion(QDataStream::Qt_5_7);
}

void Client::disconnected()
{
    qDebug() << "Disconnected!(Socket)";
    timerCounter = 0;
    timer->stop();
    timerForReConnect->start();
}

void Client::reConnectTimeout()
{
    socket->connectToHost(ip, port);
    if (socket->waitForConnected(1000))
    {
        qDebug("Socket connected to host.");
        out.setDevice(socket);
        timerForReConnect->stop();
    }else
    {
        qDebug("Socket connect fail.");
    }
}

void Client::readyRead_socket()
{
    QString data = socket->readAll();
    qDebug() << "Receive from socket << " + data;

    if(data.contains(';'))
    {
        QStringList dataList = data.split(';');
        SN = dataList.at(0);
        MAC1 = dataList.at(1);
    }
    else if(data.contains("reTest"))
    {
        reTestSignal = true;
    }
    else
    {
        qDebug() << "Error: readyRead_socket ";
        socket->flush();
        return;
    }

    timerCounter = 0;
    timer->start(1000);
    qDebug() << "Timer Start";
    socket->flush();
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
                if(controlWAutoMESProgram_PASS())
                    sendJson(testName, 1, 1);
                else
                    sendJson(testName, 1, 0);
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

    out << jsonDoc.toJson();

//    QJsonObject jsonObj;
//    jsonObj.insert("TestName", _testName);
//    jsonObj.insert("TestStage", _testStage);
//    jsonObj.insert("TestResult", _testResult);

//    QJsonDocument jsonDoc(jsonObj);

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

    sendJson("Final", 1, 0); //FAIL
//    controlWAutoMESProgram_FAIL();

    timer->stop();
    timerCounter = 0;
}
//PASS過後藉由AutoMES 過站
bool Client::controlWAutoMESProgram_PASS()
{
    WCHAR catcher[10000] = {0};
    WCHAR SN_char[100] = {0};
    WCHAR SN_Enter_char[100] = {0};
    WCHAR MAC1_char[100] = {0};
    WCHAR MAC1_Quantity_char[100] = {0};

    QString MAC1_Quantity = "1";
    MAC1_Quantity.toWCharArray(MAC1_Quantity_char);

    QString SN_Enter = SN + "{ENTER}";
    SN_Enter.toWCharArray(SN_Enter_char);

    SN.toWCharArray(SN_char);
    MAC1.toWCharArray(MAC1_char);

    //Count AutoMES resultText length
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlGetText(L"Auto Shopfloor System v1.0.9", L"",L"[CLASS:ThunderRT6TextBox; INSTANCE:1]", catcher, 10000);
    int count = QString::fromWCharArray(catcher).length();

    //Serial Number textbox
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlFocus(L"Auto Shopfloor System v1.0.9", L"", L"[CLASS:ThunderRT6TextBox; INSTANCE:3]");
    AU3_Send(SN_Enter_char);

    //Click MessageBox
    QThread::sleep(2);
    if(AU3_WinActivate(L"Message", L"") != 0)
    {
        AU3_ControlClick(L"Message", L"", L"[CLASS:Button; INSTANCE:2]", L"left", 1);

        QThread::sleep(2);
        AU3_ControlClick(L"Message", L"", L"[CLASS:Button; INSTANCE:1]", L"left", 1);
    }

    //Count AutoMES resultText length
    QThread::sleep(2);
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlGetText(L"Auto Shopfloor System v1.0.9", L"",L"[CLASS:ThunderRT6TextBox; INSTANCE:1]", catcher, 10000);

    //Check AutoMES resultText
    if(QString::fromWCharArray(catcher).length() > count)
    {
        return true;
    }else
    {
        return false;
    }
}
//FAIL過後藉由AutoMES 過站
void Client::controlWAutoMESProgram_FAIL()
{
    WCHAR catcher[10000] = {0};
    WCHAR SN_char[100] = {0};
    WCHAR MAC1_char[100] = {0};
    WCHAR SN_Enter_char[100] = {0};
    WCHAR MAC1_Quantity_char[100] = {0};

    QString MAC1_Quantity = "1";
    MAC1_Quantity.toWCharArray(MAC1_Quantity_char);

    QString SN_Enter = SN + "{ENTER}";
    SN_Enter.toWCharArray(SN_Enter_char);

    SN.toWCharArray(SN_char);
    MAC1.toWCharArray(MAC1_char);

    //Count AutoMES resultText length
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlGetText(L"Auto Shopfloor System v1.0.9", L"",L"[CLASS:ThunderRT6TextBox; INSTANCE:1]", catcher, 10000);
    int count = QString::fromWCharArray(catcher).length();

    //Serial Number textbox
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlFocus(L"Auto Shopfloor System v1.0.9", L"", L"[CLASS:ThunderRT6TextBox; INSTANCE:3]");
    AU3_Send(SN_Enter_char);

    //Click MessageBox
    QThread::sleep(2);
    if(AU3_WinActivate(L"Message", L"") != 0)
    {
        AU3_ControlClick(L"Message", L"", L"[CLASS:Button; INSTANCE:1]", L"left", 1);

        //MAC ID quantity textbox
        QThread::sleep(2);
        AU3_WinActivate(L"Manual brush bad", L"");
        AU3_ControlSetText(L"Manual brush bad", L"", L"[CLASS:Edit; INSTANCE:1]", MAC1_Quantity_char);
        AU3_ControlClick(L"Manual brush bad", L"", L"[CLASS:Button; INSTANCE:1]", L"left", 1);

        //MAC ID textbox
        QThread::sleep(2);
        AU3_WinActivate(L"Manual brush bad", L"");
        AU3_ControlSetText(L"Manual brush bad", L"", L"[CLASS:Edit; INSTANCE:1]", MAC1_char);
        AU3_ControlClick(L"Manual brush bad", L"", L"[CLASS:Button; INSTANCE:1]", L"left", 1);
    }

    //ERROR Code textbox
    QThread::sleep(2);
    AU3_WinActivate(L"Manual scan error code.", L"");
    AU3_ControlSetText(L"Manual scan error code.", L"", L"[CLASS:Edit; INSTANCE:1]", SN_char);
    AU3_ControlClick(L"Manual scan error code.", L"", L"[CLASS:Button; INSTANCE:1]", L"left", 1);

    //Count AutoMES resultText length
    QThread::sleep(2);
    AU3_WinActivate(L"Auto Shopfloor System v1.0.9", L"");
    AU3_ControlGetText(L"Auto Shopfloor System v1.0.9", L"",L"[CLASS:ThunderRT6TextBox; INSTANCE:1]", catcher, 10000);

    //Check AutoMES resultText
    if(QString::fromWCharArray(catcher).length() > count)
    {
        socket->write("test fail");
        qDebug() << "Send to socket >> test fail";
    }else
    {
        socket->write("AutoMES ERROR");
        qDebug() << "Send to socket >> AutoMES ERROR";
    }
}
