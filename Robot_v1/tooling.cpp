#include "tooling.h"
#include "ui_tooling.h"
#include "communication.h"
#include "task.h"
#include <QTcpSocket>
#include "mainwindow.h"
#include "database.h"
#include <QTimer>
#include <QTime>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QXmlStreamReader>
#include <QSqlError>
#include <QSqlQuery>

Tooling::Tooling(QWidget *parent, int _toolingNum) :
    QWidget(parent),
    toolingNumber(_toolingNum),
    ui(new Ui::Tooling)
{    
    ui->setupUi(this);
    ui->groupBox->setTitle("Tooling" + QString::number(toolingNumber));

    database = new Database(this, QString::number(toolingNumber), MainWindow::dbUrl);
    communication = new Communication(this, Communication::NO_ACK, Communication::CONNECT_TO_CLIENT);

//    將狀態連結燈號, msgBox
    connect(communication, SIGNAL(online(QString)), ui->label_light, SLOT(OnlineLighting()));
    connect(communication, SIGNAL(offline()), ui->label_light, SLOT(OfflineLighting()));
    connect(communication, SIGNAL(error(QString)), this, SLOT(errorManage(QString)));
    connect(this, SIGNAL(idle()), ui->label_light, SLOT(OnlineLighting()));
    connect(this, SIGNAL(testing()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(reTesting()), ui->label_light, SLOT(excutingLighting()));
    connect(this, SIGNAL(testFinished()), ui->label_light, SLOT(OnlineLighting()));
    connect(this, SIGNAL(error(QString)), ui->label_State, SLOT(setText(QString)));
    connect(this, SIGNAL(error(QString)), ui->label_light, SLOT(ErrorLighting()));

//    計時器初始化
    initializeClock();
    connect(&clockTimer, SIGNAL(timeout()), this, SLOT(clockUpdate()));

    connect(communication, SIGNAL(receiveMessage(QString,int,int)), this, SLOT(receiveMessage(QString,int,int)));
}

Tooling::~Tooling()
{
    delete ui;

}
//計時器初始化
void Tooling::initializeClock()
{
    clockTimer.stop();

    clockTime.setHMS(0,0,0);

    ui->clock->display(clockTime.toString("mm:ss"));
}
//測試計時用
void Tooling::clockUpdate()
{
    clockTime = clockTime.addSecs(1);

    QString clockText = clockTime.toString("mm:ss");

    if ((clockTime.second() % 2) == 0)
        clockText[2] = ' ';

    ui->clock->display(clockText);
}

void Tooling::setSocket(QTcpSocket *_socket)
{
    if(!communication->setSocket(_socket))
        return;

    updateState(IDLE);
}

//接收單項測試結果
void Tooling::receiveMessage(const QString &_testName, const int &_testStage, const int &_testResult)
{
    if(_testName == "Final")
    {
        receive_AllTestFinished(_testResult);
    }else
    {
        switch (_testStage) {
        case 0:
//            接收到測試開始
            receive_TestStart(_testName);
            break;
        case 1:
//            接收到測試結束
            receive_TestFinished(_testName, _testResult);
            break;
        default:
            break;
        }
    }
}

//接收整套測試完畢結果
void Tooling::receive_AllTestFinished(const int &_testResult)
{
    QString result;
    if(_testResult == 1){
        if(!updateState(TEST_FINISHED_PASS))
            return;
        result = "Pass";
    }
    else{
        if(!updateState(TEST_FINISHED_FAIL))
            return;
        result = "Fail";
    }
//    更新UI
    addTestUI("End test", result);
//    更新Db
    database->insert(toolingSN, toolingNumber, MO, PN, SN, MAC, "Final",dbVersion);
    database->update(result, 0, SN, "Final",dbVersion, 0);

    initialTestList();
    initializeClock();
}

//接收到測試開始
void Tooling::receive_TestStart(const QString &_testName)
{
//    更新UI
    if (updateTestUI(_testName, "testing")) {
//        更新Db
        int reTest = database->getReTest(SN, _testName, dbVersion);
        database->update("testing", currentCycleSec, SN, _testName, dbVersion, ++reTest);
    } else {
//        更新UI
        addTestUI(_testName, "testing");
//        更新Db
        database->insert(toolingSN, toolingNumber, MO, PN, SN, MAC, _testName, dbVersion);
    }
}

//接收到單項測試結束
void Tooling::receive_TestFinished(const QString &_testName, const int &_testResult)
{
    QString result;
    if(_testResult == 1)
        result = "Pass";
    else
        result = "Fail";
//    更新UI
    updateTestUI(_testName, result);
//    更新Db
    int reTest = database->getReTest(SN, _testName, dbVersion);
    database->update(result, currentCycleSec, SN, _testName, dbVersion, reTest);
}

//機台離線
void Tooling::toolDisconnect()
{
//    MainWindow::systemState = MainWindow::STOP;

    //斷線
    if(communication->state_Connection == Communication::ONLINE)
    {
        communication->closeSocket();
    }
    ui->label_State->setText("Offline");
    ui->label_light->OfflineLighting();
    state = CONNECTED;

    //刪除等待列表中此機箱的所有任務
    for(int i=0; i < MainWindow::waitingList.size(); ++i)
    {
        if(MainWindow::waitingList.at(i)->deviceNumber == toolingNumber)
        {
            Task *task = MainWindow::waitingList.takeAt(i);
            task->deleteNextAll();
        }
    }
    messageBox.setWindowTitle("info");
    messageBox.setText("Tooling " + QString::number(toolingNumber) + " is ready to move.");
    messageBox.setIcon(QMessageBox::Information);
    messageBox.show();
    disconnectFlag = false;
}

//接收SN (from ccd)
void Tooling::receiveSN(const QString &_SN, const int &_toolingNumber)
{
    if(_toolingNumber != toolingNumber)
        return;
    SN = _SN;
    ui->label_SN->setText(SN);

    getMoBySN(SN);
}

//取得Mo from WebService
void Tooling::getMoBySN(const QString &_SN)
{
    QString getUrl(MainWindow::webServiceUrl);

    QNetworkAccessManager manager;
    QNetworkReply *reply;

    QEventLoop event;
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            &event, SLOT(quit()));

    reply = manager.get(QNetworkRequest(QUrl(getUrl + "SerialNo=" + _SN + "&MC=TPMC")));

    event.exec();

    disconnect(&manager, SIGNAL(finished(QNetworkReply*)),
               &event, SLOT(quit()));

    if(reply->error())
    {
        qCritical() << "Tooling::getMoBySN() / Web service reply error.";
        MO = "N/A";
        PN = "N/A";
        return;
    }

    QByteArray answer = reply->readAll();

    QXmlStreamReader xmlReader(answer);

//    解析XML
    while (!xmlReader.atEnd()) {
        xmlReader.readNext();
        if(xmlReader.isStartElement())
        {
            if(xmlReader.name().toString() == "string")
            {
                QString readStr = xmlReader.readElementText();
                qDebug() << "Get from webService << " << readStr;
                QStringList strList = readStr.split("ΓΓ");

                MO = strList.at(1);
                PN = strList.at(2);
                return;
            }
        }
    }
    if(xmlReader.hasError())
    {
        qCritical() << "Tooling::getMoBySN() / " << xmlReader.errorString();
    }

    MO = "N/A";
    PN = "N/A";
    return;
}

//更新UI, 刷新已存在的測試項
bool Tooling::updateTestUI(const QString &_testName, const QString &_status)
{
    for(int i = 0; i < ui->treeWidget->topLevelItemCount(); i++)
    {
        if(ui->treeWidget->topLevelItem(i)->text(0) == _testName)
        {
            currentCycleSec = QTime::fromString(ui->treeWidget->topLevelItem(i)->text(2)).secsTo(clockTime);
            ui->treeWidget->topLevelItem(i)->setText(3, QString::number(currentCycleSec));
            ui->treeWidget->topLevelItem(i)->setText(1, _status);
            return true;
        }
    }
    return false;
}

//更新UI, 加入新的測試項
void Tooling::addTestUI(const QString &_testName, const QString &_status)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(QStringList() << _testName << _status << clockTime.toString());
    ui->treeWidget->insertTopLevelItem(0, item);
}

//接收MAC (from ccd)
void Tooling::receiveMAC(const QString &_MAC, const int &_toolingNumber)
{
    if(_toolingNumber != toolingNumber)
        return;
    MAC = _MAC;
    ui->label_MAC->setText(MAC);
}

//傳送SN, MAC 到 Tooling_Client端
void Tooling::sendToClient_SN_MAC()
{
    QString sendData = SN + ";" + MAC;
    communication->send(sendData, toolingNumber);
}

//執行任務(狀態防呆, PS:降寫有點醜, 應該要改用狀態機寫比較優)
void Tooling::excuteTask(const Task &_task)
{
    if(_task.deviceNumber != toolingNumber) return;
//    狀態防呆, 開始測試 -> if 第一次測試 -> TESTING   (一般測試狀態)
//                         if 第二次測試 -> RE_TESTING(重測狀態)
    if(_task.command == Task::POWER_ON)
    {
        if(testTime == ZERO_TIME)
        {
            testTime = FIRST_TIME;
            if(!updateState(TESTING)) return;
        }
        else if(testTime == FIRST_TIME)
        {
            testTime = SECOND_TIME;
            if(!updateState(RE_TESTING)) return;
        }
        else
        {
            qCritical() << "Error: Tooling::excuteTask()/Task::POWER_ON  Case exception error.";
            return;
        }
        clockTimer.start(1000);
        sendToClient_SN_MAC();
//        更新Db
        dbVersion = database->getMaxVersion(SN);
        ++dbVersion;
        database->insert(toolingSN, toolingNumber, MO, PN, SN, MAC, "Start", dbVersion);
        database->update("Pass", 0, SN, "Start", dbVersion, 0);

        emit excuteTaskByRobot(_task);
    }
//    狀態防呆, 結束測試PASS -> IDLE
//             結束測試FAIL -> if 第一次測試 -> RE_TESTING(重測狀態)
//                            if 第二次測試 -> IDLE      (機台空閒狀態)
    else if(_task.command == Task::POWER_OFF)
    {
        if(state == TEST_FINISHED_PASS)
        {
            if(!updateState(IDLE)) return;
        }
        else if(state == TEST_FINISHED_FAIL)
        {
            if(testTime == FIRST_TIME)
            {
                if(!updateState(RE_TEST)) return;
            }else if(testTime == SECOND_TIME)
            {
                if(!updateState(IDLE)) return;
            }else
            {
                qCritical() << "Error: Tooling::excuteTask()/Task::POWER_OFF/TEST_FINISHED  Case exception error.";
                return;
            }
        }
        else
        {
            qCritical() << "Error: Tooling::excuteTask()/Task::POWER_OFF  Case exception error.";
            return;
        }
        emit excuteTaskByRobot(_task);
    }
    else if(_task.command == Task::TOOL_DISCONNECTING)
    {
        if(state == IDLE || state == RE_TEST)   toolDisconnect();
        emit receiveACK(_task.ID);
        emit receiveDONE(_task.ID);
        emit waiting();
        emit addTask();
    }
}

//更新狀態(狀態防呆, PS:降寫有點醜, 應該要改用狀態機寫比較優)
bool Tooling::updateState(State _nextState)
{
    switch (_nextState) {
    case CONNECTED:

        break;
    case IDLE:
        if(state != CONNECTED && state != TEST_FINISHED_PASS && state != TEST_FINISHED_FAIL)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict0");
            return false;
        }

        if(disconnectFlag)
        {
            Task::createAction(Task::TOOL_DISCONNECT, toolingNumber);
        }else
        {
            ui->disconnectButton->setEnabled(true);
            testTime = ZERO_TIME;
            Task::createAction(Task::GET_DUT_TO_TEST, toolingNumber);
        }

        emit idle();
        ui->label_State->setText("Idle");
        emit addTask();

        break;
    case TESTING:
        if(state != IDLE)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict1");
            return false;
        }
        emit testing();
        ui->label_State->setText("Testing");
        break;
    case TEST_FINISHED_PASS:
        if(state != TESTING && state != RE_TESTING)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict2");
            return false;
        }
        emit testFinished();
        ui->label_State->setText("TestFinished");

        Task::createAction(Task::TEST_FINISHED_TO_PASS, toolingNumber);
        emit addTask();

        break;
    case TEST_FINISHED_FAIL:
        if(state != TESTING && state != RE_TESTING)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict3");
            return false;
        }
        emit testFinished();
        ui->label_State->setText("TestFinished");

        if(testTime == FIRST_TIME)
        {
            Task::createAction(Task::TEST_FAIL_TO_RETEST, toolingNumber);
        }else if(testTime == SECOND_TIME)
        {
            Task::createAction(Task::TEST_FINISHED_TO_FAIL, toolingNumber);
        }
        else
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict4");
            return false;
        }
        emit addTask();

        break;
    case RE_TEST:
        if(state != TEST_FINISHED_FAIL)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict5");
            return false;
        }
        emit reTest();
        ui->label_State->setText("ReTest");
        break;
    case RE_TESTING:
        if(state != RE_TEST)
        {
            errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict6");
            return false;
        }
        emit reTesting();
        ui->label_State->setText("ReTesting");
        break;
    default:
        errorManage("Error: Tooling" + QString::number(toolingNumber) + " state transfer conflict7");
        return false;
    }

    state = _nextState;
    return true;
}

//紀錄 TestLog
void Tooling::initialTestList()
{
    QFile logFile(logPath + "\\" + SN + ".log");
    logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&logFile);
    out << "Tooling Number : " << QString::number(toolingNumber) << endl
        << "MAC Number     : " << MAC << endl
        << "Test time spend: " << clockTime.toString("mm:ss") << " (mins/secs)"<<endl
        << "------------------------------"<< endl;

    for(int i = ui->treeWidget->topLevelItemCount()-1; i >= 0; i--)
        out << "Test item:" << ui->treeWidget->topLevelItem(i)->text(0)
            << "\tResult:" << ui->treeWidget->topLevelItem(i)->text(1)
            << "\tSpend time:" << ui->treeWidget->topLevelItem(i)->text(3) << "(sec)" << endl;

    ui->treeWidget->clear();
}

void Tooling::errorManage(const QString &_str)
{
    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setWindowTitle("Error");
    messageBox.setText(_str);
    messageBox.show();
    qCritical() << _str;
    state = ERROR;
    emit error(_str);
}

void Tooling::on_disconnectButton_clicked()
{
    disconnectFlag = true;
    ui->disconnectButton->setEnabled(false);
}
