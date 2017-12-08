#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>

class Tooling;
class QTcpServer;
class QTcpSocket;
class Task;
class TaskManager;
class DynamicSetting;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
//    系統狀態
    enum SystemState{ START, STOP };
    enum SystemAction{ INACTION, WAITING };
    enum TrayMode{ TWO_TYPES = 0, ONE_TYPE};

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static QList<Task*> waitingList;
    static QList<Task*> inActionList;
    static QList<Task*> doneList;
//    系統狀態
    static SystemState systemState;
    static SystemAction systemAction;
//    機箱的數量
    static int toolingQuantity;
//    入料區的模式, 單機種 或是 雙機種
    static TrayMode trayMode;

protected:
    void closeEvent(QCloseEvent *event);

signals:
    void systemStart();
    void systemStop();

private slots:
    void newConnection();
    void readyRead();
    void serverError();

    void setting();
    void about();
    void help();
//    在任務列表間搬移
    void moveTask_Wait_To_InAciton(const QString &_ID);
    void moveTask_InAction_To_Done(const QString &_ID);
//    更新tableWidget Gui 介面
    void updateAllTableWidget();
    void updateWaitingTable();
    void updateInActionTable();
    void updateDoneTable();

    void on_Button_Start_clicked();
    void on_Button_Stop_clicked();
    void on_Button_Quit_clicked();
private:
    Ui::MainWindow *ui;
//    建立Gui介面
    void createActions();
    void createMenus();
//    建立預刷視窗
    void createPreScanDialog();

    void readSettingFile();
//    初始化各設備
    void initialToolings();
    void initialRobot();
    void initialCcd();
    void initialTaskManager();
//    開啟Server
    void serverOnline();
//    將完成任務記錄到Log
    void DoneTaskToLog();
//    清除多餘的DoneList任務
    void cleanDoneList();
//    檢查各設備連線狀態
    bool checkDeviceAllConnected();

    QList<Tooling*> tooling;
    TaskManager *taskManager;
    QAction *settingAction;
    QAction *aboutAction;
    QAction *helpAction;
    QMenu *optionMenu;
    QTcpServer *server;
    QTcpSocket *socket;
    int port, robotPort;
    QString robotIP, toolingLogPath, robotLogPath;
    QDialog *preScanDialog;
    DynamicSetting *dynamicSetting;
};

#endif // MAINWINDOW_H
