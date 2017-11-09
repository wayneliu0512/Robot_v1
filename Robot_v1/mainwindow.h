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

    enum SystemState{ START, STOP };
    enum SystemAction{ INACTION, WAITING };

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    static QList<Task*> waitingList;
    static QList<Task*> inActionList;
    static QList<Task*> doneList;
    static SystemState systemState;
    static SystemAction systemAction;
    static int toolingQuantity;

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

    void moveTask_Wait_To_InAciton(const QString &_ID);
    void moveTask_InAction_To_Done(const QString &_ID);
    void updateAllTableWidget();
    void updateWaitingTable();
    void updateInActionTable();
    void updateDoneTable();

    void on_Button_Start_clicked();
    void on_Button_Stop_clicked();
    void on_Button_Quit_clicked();

private:
    Ui::MainWindow *ui;

    void createActions();
    void createMenus();
    void createPreScanDialog();

    void readSettingFile();

    void initialToolings();
    void initialRobot();
    void initialCcd();
    void initialTaskManager();

    void serverOnline();

    void DoneTaskToLog();
    void cleanDoneList();

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
