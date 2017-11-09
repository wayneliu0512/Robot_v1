#ifndef TASK_H
#define TASK_H

#include <QString>

class Task
{

public:
    enum Action{ GET_DUT_TO_TEST, TEST_FINISHED_TO_PASS, TEST_FINISHED_TO_FAIL, TEST_FAIL_TO_RETEST, SCAN_ERROR_TO_FAIL,
                 UPDATE_TOOLINGS_BASE};
    enum Command{ TO_SCAN_SN, TO_SCAN_MAC, START_SCAN, TO_SCAN_FAIL, TO_TOOLING, REPUT_TOOLING, TO_PASS, TO_FAIL, POWER_OFF,
                  POWER_ON, UPDATE_TRAY, UPDATE_BASE};
    enum Device{ ROBOT, TOOLING, CCD};

    Task(Task::Command _command = TO_SCAN_SN, Task::Device _device = ROBOT, int _deviceNumber = 0, Task *_nextTask = nullptr);

    ~Task();

    static void createAction(Action _action, int _deviceNumber = 0);
    static Task *getAction(Action _action, int _deviceNumber = 0);

    QString commandToString();
    QString deviceToString();

    Task *next(Task *_nextTask);
    Task *takeNextTask();
    void deleteNextAll();

    QString ID;
    Command command;
    Device device;
    Task *nextTask;

    int deviceNumber;
};

#endif // TASK_H
