#ifndef CCD_H
#define CCD_H

#include <QWidget>

class QTcpSocket;
class Communication;
class Task;

namespace Ui {
class Ccd;
}

class Ccd : public QWidget
{
    Q_OBJECT

public:
    enum State{ CONNECTED, SHOOTING, WAITING};

    explicit Ccd(QWidget *parent = 0);
    ~Ccd();

    void setSocket(QTcpSocket *_socket);

    State state = CONNECTED;

    Communication *communication;

signals:
    void connected();
    void shooting();
    void waiting();

    void sendSN(const QString &_SN, const int &_ToolingNumber);
    void sendMAC(const QString &_MAC, const int &_ToolingNumber);
public slots:
    void excuteTask(const Task &_task);

private slots:
    void actionFinished();
    void receiveError(const QString &_ID);
    void receiveSN(const QString &_SN);
    void receiveMAC(const QString &_MAC);

private:
    Ui::Ccd *ui;

    void updateState(State _state);
    int ToolingNumber = 0;
};

#endif // CCD_H
