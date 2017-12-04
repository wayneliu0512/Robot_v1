#ifndef PRESCANGROUPBOX_H
#define PRESCANGROUPBOX_H
#include <QWidget>
#include <QMessageBox>

/* GroupBox
   機箱預刷系統, 每個機箱刷入項目：
   1.Tooling    -> 機箱序號
   2.Base       -> Base序號
   3.ModuleBase -> 模組座序號
   4.Module     -> 模組序號

   以上刷入項目, 有彼此綁定規則限制, 依照這些規則可以做到防呆, 提醒刷入錯誤
*/

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

    void on_lineEdit_ElecBox_returnPressed();

private:
    Ui::PreScanGroupBox *ui;

    void keyInFinished();

    QMessageBox messageBox;
    int toolingNumber;
    DynamicSetting *setting;
};

#endif // PRESCANGROUPBOX_H
