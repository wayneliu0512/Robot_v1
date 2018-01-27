#ifndef TASK_H
#define TASK_H
#include <QString>

/*任務：
  Action 裡面包含一連串的Command
  例如: GET_DUT_TO_TEST{ ToScanSN->StartScan->ToScanMAC->StartScan->ToTooling->PowerOn->UpdateTray }
*/

class Task
{
public:
    enum Action{ GET_DUT_TO_TEST, TEST_FINISHED_TO_PASS, TEST_FINISHED_TO_FAIL, TEST_FAIL_TO_RETEST, SCAN_ERROR_TO_FAIL,
                 UPDATE_TOOLINGS_BASE, UPDATE_ALLSET};
    enum Command{ TO_SCAN_SN, TO_SCAN_MAC, START_SCAN, TO_SCAN_FAIL, TO_TOOLING, REPUT_TOOLING, TO_PASS, TO_FAIL, POWER_OFF,
                  POWER_ON, UPDATE_TRAY, UPDATE_BASE, UPDATE_SETTING};
    enum Device{ ROBOT, TOOLING, CCD};

    Task(Task::Command _command = TO_SCAN_SN, Task::Device _device = ROBOT, int _deviceNumber = 0);

    ~Task();
//    創建任務, 創建成功會加入waitingList
    static void createAction(Action _action, int _deviceNumber = 0);
//    創建任務, 創建成功會 return Task*
    static Task *getAction(Action _action, int _deviceNumber = 0);

    QString commandToString();
    QString deviceToString();
//    連結下一個任務
    Task *next(Task *_nextTask);
//    擷取出下一個任務
    Task *takeNextTask();
//    刪除接下來連結的任務
    void deleteNextAll();

    QString ID;     // 每個任務都會生成一個獨一無二的ID, 由此ID來確保ACK機制任務是正確的
    Command command;
    Device device;
    Task *nextTask;

    int deviceNumber, tray_mode;
};

#endif // TASK_H
