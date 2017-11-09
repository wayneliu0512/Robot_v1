#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QTcpSocket;
class QTcpServer;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
    void output(const QString &_str);

private:
    Ui::MainWindow *ui;

private slots:
    void newConnection();
    void readyRead();
    void serverError();
    void socketError();
    void sendDone();
    void on_pushButton_Clear_clicked();

private:
    QTcpServer *server;
    QTcpSocket *socket;
    QStringList ID_List;
};

#endif // MAINWINDOW_H
