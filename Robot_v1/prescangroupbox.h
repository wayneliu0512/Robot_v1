#ifndef PRESCANGROUPBOX_H
#define PRESCANGROUPBOX_H

#include <QWidget>
#include <QMessageBox>

class DynamicSetting;

namespace Ui {
class PreScanGroupBox;
}

class PreScanGroupBox : public QWidget
{
    Q_OBJECT

public:
    enum LineEdit{ TOOLING, BASE, MODULE_BASE, MODULE };

    explicit PreScanGroupBox(QWidget *parent = 0, const int &_toolingNumber = 0,
                             DynamicSetting *_setting = 0);
    ~PreScanGroupBox();
signals:
    void finished();
    void allComplete();
public slots:
    void StartEdit();
    void ReKeyLineEdit(const LineEdit &_lineEdit);

private slots:

    void on_lineEdit_Tooling_returnPressed();

    void on_lineEdit_Base_returnPressed();

    void on_lineEdit_ModuleBase_returnPressed();

    void on_lineEdit_Module_returnPressed();

private:
    Ui::PreScanGroupBox *ui;

    void keyInFinished();

    QMessageBox messageBox;
    int toolingNumber;
    DynamicSetting *setting;
};

#endif // PRESCANGROUPBOX_H
