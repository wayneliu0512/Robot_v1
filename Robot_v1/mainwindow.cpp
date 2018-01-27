#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QKeySequence>
#include <QDebug>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QSettings>
#include <QWindow>
#include <QCloseEvent>
#include <QDialogButtonBox>
#include <QScrollArea>
#include <QDir>
#include "dynamicsetting.h"
#include "tooling.h"
#include "task.h"
#include "communication.h"
#include "taskmanager.h"
#include "prescangroupbox.h"
#include "light.h"
#include "base.h"

QList<Task*> MainWindow::waitingList;
QList<Task*> MainWindow::inActionList;
QList<Task*> MainWindow::doneList;
MainWindow::SystemState MainWindow::systemState = MainWindow::STOP;
MainWindow::SystemAction MainWindow::systemAction = MainWindow::WAITING;
MainWindow::TrayMode MainWindow::trayMode = MainWindow::TWO_TYPES;
QString MainWindow::dbUrl = "";
QString MainWindow::webServiceUrl = "";

int MainWindow::toolingQuantity;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->gridLayout->setSpacing(20);

    readSettingFile();

    dynamicSetting = new DynamicSetting(this, toolingQuantity);

    createActions();
    createMenus();
    createPreScanDialog();

    initialToolings();
    initialRobot();
    initialCcd();
    initialTaskManager();

    serverOnline();

//  preScanDialog->exec();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(QMessageBox::warning(this, "Warning", "Do you want to quit the application ?",
                            QMessageBox::Ok, QMessageBox::Cancel) == QMessageBox::Cancel)
    {
        event->ignore();
    }else
    {
        systemState = STOP;
        QThread::msleep(500);
        qDebug() << "Quit Application";

        while(!waitingList.isEmpty()){
            Task *task = waitingList.takeFirst();
            task->deleteNextAll();
        }
        qDebug() << "Delete Waiting list";

        while(!inActionList.isEmpty()){
            Task *task = inActionList.takeFirst();
            task->deleteNextAll();
        }
        qDebug() << "Delete inAction list";

        while(!doneList.isEmpty()){
            Task *task = doneList.takeFirst();
            task->deleteNextAll();
        }
        qDebug() << "Delete Done list";
        event->accept();
    }
}

void MainWindow::readSettingFile()
{
#if defined(Q_OS_MACOS)
    QFile base_settngINI("/Users/wayneliu/Documents/Qt/build-Robot_v1-Desktop_Qt_5_8_0_clang_64bit-Release/Base_Setting.ini");
    QFile module_setting("/Users/wayneliu/Documents/Qt/build-Robot_v1-Desktop_Qt_5_8_0_clang_64bit-Release/Module_Setting.ini");
#elif defined(Q_OS_WIN)
    QFile base_settngINI("Base_Setting.ini");
    QFile module_setting("Module_Setting.ini");
#endif

    if(!base_settngINI.exists()){
        QMessageBox::critical(this, "Read file Error", "Error: Couldn't find Base_Setting.ini.");
        qWarning() << "Error: Couldn't find Base_Setting.ini";
        return;
    }

    if(!module_setting.exists()){
        QMessageBox::critical(this, "Read file Error", "Error: Couldn't find Module_Setting.ini");
        qWarning() << "Error: Couldn't find Module_Setting.ini";
        return;
    }

#if defined(Q_OS_MACOS)
    QSettings setting("/Users/wayneliu/Documents/Qt/build-Robot_v1-Desktop_Qt_5_8_0_clang_64bit-Release/Setting.ini", QSettings::IniFormat);
#elif defined(Q_OS_WIN)
    QSettings setting("Setting.ini", QSettings::IniFormat);
#endif

    if(setting.value("Option/Port") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/Port");
        qWarning() << "Error: We couldn't read Setting.ini Option/Port";
        exit(1);
    }else{
        port = setting.value("Option/Port").toInt();
    }

    if(setting.value("Option/RobotIP") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/RobotIP");
        qWarning() << "Error: We couldn't read Setting.ini Option/RobotIP";
        exit(1);
    }else{
        robotIP = setting.value("Option/RobotIP").toString();
    }

    if(setting.value("Option/RobotPort") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/RobotPort");
        qWarning() << "Error: We couldn't read Setting.ini Option/RobotPort";
        exit(1);
    }else{
        robotPort = setting.value("Option/RobotPort").toInt();
    }

    if(setting.value("Option/DbUrl") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/DbUrl");
        qWarning() << "Error: We couldn't read Setting.ini Option/DbUrl";
        exit(1);
    }else{
        dbUrl = setting.value("Option/DbUrl").toString();
    }

    if(setting.value("Option/WebServiceUrl") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/WebServiceUrl");
        qWarning() << "Error: We couldn't read Setting.ini Option/WebServiceUrl";
        exit(1);
    }else{
        webServiceUrl = setting.value("Option/WebServiceUrl").toString();
    }

    if(setting.value("Option/ToolingLogPath") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/ToolingLogPath");
        qWarning() << "Error: We couldn't read Setting.ini Option/ToolingLogPath";
        exit(1);
    }else{
        QString path = setting.value("Option/ToolingLogPath").toString();
    #if defined(Q_OS_MACOS)

    #elif defined(Q_OS_WIN)
        QDir dir(path);
        if(!dir.exists())
        {
            QMessageBox::critical(this, "Read file Error", "Error: We couldn't find" + path);
            exit(1);
        }
    #endif
        toolingLogPath = path;
    }

    if(setting.value("Option/RobotLogPath") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/RobotLogPath");
        qWarning() << "Error: We couldn't read Setting.ini Option/RobotLogPath";
        exit(1);
    }else{
        QString path = setting.value("Option/RobotLogPath").toString();
    #if defined(Q_OS_MACOS)

    #elif defined(Q_OS_WIN)
        QDir dir(path);
        if(!dir.exists())
        {
            QMessageBox::critical(this, "Read file Error", "Error: We couldn't find" + path);
            exit(1);
        }
    #endif
        robotLogPath = path;
    }

    if(setting.value("Option/ToolingQuantity") == QVariant())
    {
        QMessageBox::critical(this, "Read file Error", "Error: We couldn't read Setting.ini Option/ToolingQuantity");
        qWarning() << "Error: We couldn't read Setting.ini Option/ToolingQuantity";
        exit(1);
    }else{
        toolingQuantity = setting.value("Option/ToolingQuantity").toInt();
    }

    if(toolingQuantity > 9)
    {
        QMessageBox::critical(this, "Setting Error", "Error: Tooling quantity over max number.");
        qWarning() << "Error: Tooling quantity over max number.";
        exit(1);
    }
}

void MainWindow::createActions()
{
    settingAction = new QAction(tr("&Setting"), this);
    settingAction->setStatusTip("Setting each device numbers or parameter.");
    connect(settingAction, SIGNAL(triggered(bool)), this, SLOT(setting()));

    helpAction = new QAction(tr("&Help"), this);
    helpAction->setShortcut(QKeySequence::HelpContents);
    helpAction->setStatusTip("Help you solve your problem.");
    connect(helpAction, SIGNAL(triggered(bool)), this, SLOT(help()));

    aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip("There are some information about this program");
    connect(aboutAction, SIGNAL(triggered(bool)), this, SLOT(about()));
}

void MainWindow::setting()
{
    qDebug() << "Setting";
}

void MainWindow::help()
{
    qDebug() << "Help";
}

void MainWindow::about()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::createMenus()
{
    optionMenu = menuBar()->addMenu(tr("&Option"));
    optionMenu->addAction(settingAction);
    optionMenu->addAction(helpAction);
    optionMenu->addAction(aboutAction);
}

void MainWindow::createPreScanDialog()
{
    QList<PreScanGroupBox*> groupBoxList;
    QGridLayout *gridLayout = new QGridLayout;

    int n = 0;
    for(int i = 0; i < toolingQuantity; i++)
    {
        groupBoxList.append(new PreScanGroupBox(this, i+1, dynamicSetting));

        if(i%4 == 0)
            n++;

        gridLayout->addWidget(groupBoxList.at(i), n, i%4, 1, 1);
    }

    for(int i = 0; i < toolingQuantity-1; i++)
        connect(groupBoxList.at(i), SIGNAL(finished()), groupBoxList.at(i+1),SLOT(StartEdit()));        

    preScanDialog = new QDialog(this);
    preScanDialog->setLayout(gridLayout);
    preScanDialog->setWindowTitle("PreScan Barcode");

    for(int i = 0; i < toolingQuantity; i++)
        connect(groupBoxList.at(i), SIGNAL(allComplete()), preScanDialog, SLOT(accept()));
}

void MainWindow::initialToolings()
{   
    QGridLayout *toolingGridLayout = new QGridLayout;

    int n = 0;
    for(int i = 0; i < toolingQuantity; i++)
    {
        tooling.append(new Tooling(this));
        tooling.at(i)->setToolingNumber(i+1);
        tooling.at(i)->setLogPath(toolingLogPath);
        connect(tooling.at(i), SIGNAL(addTask()), this, SLOT(updateWaitingTable()));
        connect(tooling.at(i), SIGNAL(excuteTaskByRobot(Task)), ui->robot, SLOT(excuteTask(Task)));

        if(i%3 == 0)
             n++;

        toolingGridLayout->addWidget(tooling.at(i), n, i%3, 1, 1);
    }
    ui->scrollAreaWidgetContents->setLayout(toolingGridLayout);

   // 加入 scrollBar
}

void MainWindow::initialRobot()
{
    ui->robot->setPortIP(robotPort, robotIP);
    ui->robot->connectToRobot();
    connect(ui->robot, SIGNAL(addTask()), this, SLOT(updateWaitingTable()));
    connect(ui->robot->communication, SIGNAL(receiveACK(QString)), this, SLOT(moveTask_Wait_To_InAciton(QString)));
    connect(ui->robot->communication, SIGNAL(receiveDONE(QString)), this, SLOT(moveTask_InAction_To_Done(QString)));
    connect(this, SIGNAL(systemStop()), ui->robot, SLOT(aboutToStop()));
}

void MainWindow::initialCcd()
{
    connect(ui->ccd->communication, SIGNAL(receiveACK(QString)), this, SLOT(moveTask_Wait_To_InAciton(QString)));
    connect(ui->ccd->communication, SIGNAL(receiveDONE(QString)), this, SLOT(moveTask_InAction_To_Done(QString)));

    for(int i = 0; i < toolingQuantity; i++)
    {
        connect(ui->ccd, SIGNAL(sendSN(QString,int)), tooling.at(i), SLOT(receiveSN(QString,int)));
        connect(ui->ccd, SIGNAL(sendMAC(QString,int)), tooling.at(i), SLOT(receiveMAC(QString,int)));
    }
}

void MainWindow::initialTaskManager()
{
    taskManager = new TaskManager(this);

    connect(this, SIGNAL(systemStart()), taskManager, SLOT(start()));
    connect(ui->robot, SIGNAL(waiting()), taskManager, SLOT(systemActionFinished()));
    connect(ui->ccd, SIGNAL(waiting()), taskManager, SLOT(systemActionFinished()));

    for(int i = 0; i < toolingQuantity; i++)
    {
//        connect(tooling.at(i), SIGNAL(addTask()), taskManager, SLOT(startRunningTasks()));
        connect(taskManager, SIGNAL(excuteToolingTask(Task)), tooling.at(i), SLOT(excuteTask(Task)));
    }

    connect(taskManager, SIGNAL(excuteRobotTask(Task)), ui->robot, SLOT(excuteTask(Task)));
    connect(taskManager, SIGNAL(excuteCcdTask(Task)), ui->ccd, SLOT(excuteTask(Task)));
}

void MainWindow::serverOnline()
{
    server = new QTcpServer(this);
    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));
    connect(server, SIGNAL(acceptError(QAbstractSocket::SocketError)), this, SLOT(serverError()));

    if(!server->listen(QHostAddress::Any, port))
    {
        QMessageBox::critical(this, "Error", "Server online error.");
        exit(1);
    }
    else
    {
        qDebug() << "Server Start!";
    }
}

void MainWindow::newConnection()
{
    socket = server->nextPendingConnection();
    qDebug() << "New connection IP: " + socket->peerAddress().toString();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
}

void MainWindow::readyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());

    QString stringFromClient;

    stringFromClient = socket->readAll();

    qDebug() << "MainWindow::readyRead() read << " + stringFromClient;

    QRegExp rx("^tooling[1-" + QString::number(toolingQuantity) + "]{1,1}$");
    if(rx.exactMatch(stringFromClient))
    {
        tooling.at(stringFromClient.mid(7).toInt()-1)->setSocket(socket);
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
    else if(stringFromClient == "ccd")
    {
        ui->ccd->setSocket(socket);
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
    }
    else
    {
        qWarning() << "MainWindow::readyRead() exception read << " + stringFromClient;
        disconnect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));
        delete socket;
    }
}

void MainWindow::serverError()
{
    QMessageBox::critical(this, "Server error", server->errorString());
    qCritical() << "Server error";
}

void MainWindow::moveTask_Wait_To_InAciton(const QString &_ID)
{
    //比對任務ID,將等待列表內的任務丟到處理中列表
    for(int i = 0; i < waitingList.length(); i++)
    {
        if(_ID == waitingList.at(i)->ID)
            inActionList.append(waitingList.takeAt(i));
            updateAllTableWidget();
    }
}

void MainWindow::moveTask_InAction_To_Done(const QString &_ID)
{
    //比對任務ID,將處理中列表內的任務丟到動作完畢列表
    for(int i = 0; i < inActionList.length(); i++)
    {
        if(_ID == inActionList.at(i)->ID)
        {
            doneList.append(inActionList.takeAt(i));
            cleanDoneList();
            DoneTaskToLog();
            updateAllTableWidget();
        }
    }
}

void MainWindow::cleanDoneList()
{
    if(doneList.length() > 20)
    {
        Task *task = doneList.takeFirst();
        task->deleteNextAll();
    }
}

void MainWindow::DoneTaskToLog()
{
#if defined(Q_OS_MACOS)
#elif defined(Q_OS_WIN)
    Task *task = doneList.last();
    QFile logFile(robotLogPath + "\\" + QDate::currentDate().toString(Qt::ISODate) + "_RobotLog.log");
    if(logFile.exists())
    {
        logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
    }else
    {
        logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    }
    QTextStream out(&logFile);
    out << "Time         : " << QTime::currentTime().toString() << endl
        << "Command      : " << task->commandToString() << endl
        << "ToolingNumber: " << QString::number(task->deviceNumber) << endl
        << "-------------------------------------------------------" << endl;
#endif
}

void MainWindow::updateAllTableWidget()
{
    updateWaitingTable();
    updateInActionTable();
    updateDoneTable();
}

void MainWindow::updateWaitingTable()
{
    ui->tableWidget_Waiting->setRowCount(0);
    if(waitingList.empty())
        return;

    foreach(Task *task, waitingList)
    {
        QTableWidgetItem *CommandItem = new QTableWidgetItem(task->commandToString());
        CommandItem->setFlags(CommandItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceItem = new QTableWidgetItem(task->deviceToString());
        DeviceItem->setFlags(DeviceItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceNumberItem = new QTableWidgetItem(QString::number(task->deviceNumber));
        DeviceNumberItem->setFlags(DeviceNumberItem->flags() ^ Qt::ItemIsEditable);
        int row = ui->tableWidget_Waiting->rowCount();
        ui->tableWidget_Waiting->insertRow(row);
        ui->tableWidget_Waiting->setItem(row, 0, CommandItem);
        ui->tableWidget_Waiting->setItem(row, 1, DeviceItem);
        ui->tableWidget_Waiting->setItem(row, 2, DeviceNumberItem);
    }
}

void MainWindow::updateInActionTable()
{
    ui->tableWidget_InAction->setRowCount(0);
    if(inActionList.empty())
        return;

    foreach(Task *task, inActionList)
    {
        QTableWidgetItem *CommandItem = new QTableWidgetItem(task->commandToString());
        CommandItem->setFlags(CommandItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceItem = new QTableWidgetItem(task->deviceToString());
        DeviceItem->setFlags(DeviceItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceNumberItem = new QTableWidgetItem(QString::number(task->deviceNumber));
        DeviceNumberItem->setFlags(DeviceNumberItem->flags() ^ Qt::ItemIsEditable);
        int row = ui->tableWidget_InAction->rowCount();
        ui->tableWidget_InAction->insertRow(row);
        ui->tableWidget_InAction->setItem(row, 0, CommandItem);
        ui->tableWidget_InAction->setItem(row, 1, DeviceItem);
        ui->tableWidget_InAction->setItem(row, 2, DeviceNumberItem);
    }
}

void MainWindow::updateDoneTable()
{
    ui->tableWidget_Done->setRowCount(0);
    if(doneList.empty())
    {
        return;
    }
    foreach(Task *task, doneList)
    {
        QTableWidgetItem *CommandItem = new QTableWidgetItem(task->commandToString());
        CommandItem->setFlags(CommandItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceItem = new QTableWidgetItem(task->deviceToString());
        DeviceItem->setFlags(DeviceItem->flags() ^ Qt::ItemIsEditable);
        QTableWidgetItem *DeviceNumberItem = new QTableWidgetItem(QString::number(task->deviceNumber));
        DeviceNumberItem->setFlags(DeviceNumberItem->flags() ^ Qt::ItemIsEditable);
        int row = ui->tableWidget_Done->rowCount();
        ui->tableWidget_Done->insertRow(row);
        ui->tableWidget_Done->setItem(row, 0, CommandItem);
        ui->tableWidget_Done->setItem(row, 1, DeviceItem);
        ui->tableWidget_Done->setItem(row, 2, DeviceNumberItem);
    }
}

void MainWindow::on_Button_Start_clicked()
{
    if(systemState == START)
        return;

//    if(!checkDeviceAllConnected())
//        return;

    if(preScanDialog == nullptr)
        createPreScanDialog();

    while(1)
    {
        int result = preScanDialog->exec();

        if(result == 0)
        {
            if(QMessageBox::Ok == QMessageBox::warning(preScanDialog, "Warning",
                                                       "Are you sure you don't want to update Base_setting",
                                                       QMessageBox::Cancel, QMessageBox::Ok))
            {
                qDebug() << "Use old Base_setting";
                dynamicSetting->backToOriginalSetting();
//                if(toolingQuantity >= 3)
//                    dynamicSetting->adjustTooling3Offset();
                break;
            }
        }else if(result == 1)
        {
            dynamicSetting->updateSetting();
//            if(toolingQuantity >= 3)
//                dynamicSetting->adjustTooling3Offset();
            break;
        }
    }
    delete preScanDialog;
    preScanDialog = nullptr;

    ui->robot->setBase(dynamicSetting->nowOffsetList);
    ui->robot->setElecBoxList(dynamicSetting->nowElectrostaticBoxList);
    ui->robot->updateAllSet();

    for(int i = 0; i < toolingQuantity; ++i)
    {
        tooling.at(i)->setToolingSN(dynamicSetting->nowOffsetList->at(i).SN);
    }

    QMessageBox::warning(this, "Warning",
                         "Please make sure keep personnel clearance and free of obstacle!"
                         "\nRobot is about to take action!");

    systemState = START;
    emit systemStart();
    qDebug() << "emit systemStart()";
}

void MainWindow::on_Button_Stop_clicked()
{
    if(systemState == STOP)
        return;
    systemState = STOP;
    emit systemStop();
    qDebug() << "emit systemStop()";
}

bool MainWindow::checkDeviceAllConnected()
{
    //檢查手臂連線狀態
    while(ui->robot->communication->state_Connection == Communication::OFFLINE)
    {
        if(QMessageBox::critical(this, tr("Error"), tr("Robot connection error\nDo you want to reconnect?"),
                                 QMessageBox::Ok,QMessageBox::Cancel) == QMessageBox::Ok)
        {
            ui->robot->connectToRobot();
            qDebug() << "reConnect to robot";
        }else{
            return false;
        }
    }

    //檢查CCD連線狀態
    if(ui->ccd->communication->state_Connection == Communication::OFFLINE)
    {
        QMessageBox::critical(this, tr("Error"), tr("CCD connection error"),QMessageBox::Ok);
        qDebug() << "Ccd connection error";
        return false;
    }

    for( int i = 0; i < tooling.length(); i++)
    {
        if(tooling.at(i)->communication->state_Connection == Communication::OFFLINE)
        {
            QMessageBox::critical(this, tr("Error"), tr("Tooling connection error"),QMessageBox::Ok);
            qDebug() << "Tooling connection error";
            return false;
        }
    }

    return true;
}

void MainWindow::on_Button_Quit_clicked()
{
    close();
}
