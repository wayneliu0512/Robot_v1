#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QObject>
#include <QTcpSocket>
#include <QAbstractSocket>
#include <QtCore>
#include <QSerialPort>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

signals:
    void output(const QString &_str);

public slots:

    void connected();
    void disconnected();
    void readyRead_socket();

private slots:

    void on_pushButton_Connect_clicked();

    void on_pushButton_Disconnect_clicked();

    void on_pushButton_Clear_clicked();

private:
    Ui::Widget *ui;

    QTcpSocket *socket;

    QJsonValue ID_value;

    void DONE_Respond();

    void initialLineEdit();
};

#endif // WIDGET_H
