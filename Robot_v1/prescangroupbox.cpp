#include "prescangroupbox.h"
#include "ui_prescangroupbox.h"
#include "base.h"
#include "dynamicsetting.h"
#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>

PreScanGroupBox::PreScanGroupBox(QWidget *parent, const int &_toolingNumber, DynamicSetting *_setting) :
    QWidget(parent),toolingNumber(_toolingNumber), setting(_setting),
    ui(new Ui::PreScanGroupBox)
{
    ui->setupUi(this);

    ui->groupBox->setTitle("Tooling" + QString::number(toolingNumber));

    messageBox.setIcon(QMessageBox::Critical);
    messageBox.setWindowTitle("Error");
}

PreScanGroupBox::~PreScanGroupBox()
{
    delete ui;
}

void PreScanGroupBox::keyInFinished()
{
    if(ui->lineEdit_Tooling->text().isEmpty())
    {
        ui->lineEdit_Tooling->setFocus();
        return;
    }

    if(ui->lineEdit_Base->text().isEmpty())
    {
        ui->lineEdit_Base->setFocus();
        return;
    }

    if(ui->lineEdit_ModuleBase->text().isEmpty())
    {
        ui->lineEdit_ModuleBase->setFocus();
        return;
    }

    if(ui->lineEdit_Module->text().isEmpty())
    {
        ui->lineEdit_Module->setFocus();
        return;
    }

    if(ui->lineEdit_ElecBox->text().isEmpty())
    {
        ui->lineEdit_ElecBox->setFocus();
        return;
    }

    Base offset;

    //檢查ini檔裡是否有此序號
    for(int i = 0 ; i < setting->offsetList->length() ; i++)
    {
        if(setting->offsetList->at(i).SN == ui->lineEdit_Tooling->text())
        {

            offset = setting->offsetList->at(i);
            break;
        }
    }
    //檢查ini檔裡是否有此序號
    if(offset.SN.isEmpty())
    {
        messageBox.setText("There's no tooling SN to match.");
        messageBox.show();
        qDebug() << "PreScanDialog: There's no tooling SN to match.";
        ui->lineEdit_Tooling->clear();
        ui->lineEdit_Tooling->setFocus();
        return;
    }
    //檢查模組座位置跟base位置是否吻合
    if(ui->lineEdit_ModuleBase->text().section('-', 1, 1) != ui->lineEdit_Base->text().section('-', 2, 2))
    {
        messageBox.setText("Module base can't match base.");
        messageBox.show();
        qDebug() << "PreScanDialog: Module base can't match base.";
        ui->lineEdit_ModuleBase->clear();
        ui->lineEdit_ModuleBase->setFocus();
        return;
    }
    //檢查模組機種是否與機箱機種吻合
    if(!ui->lineEdit_Module->text().section('-', 1, 1).contains(offset.type))
    {
        messageBox.setText("Module type can't match tooling type.");
        messageBox.show();
        qDebug() << "PreScanDialog: Module type can't match tooling type.";
        ui->lineEdit_Module->clear();
        ui->lineEdit_Module->setFocus();
        return;
    }
    //檢查每組機箱序號沒有重複到
    for(int i = 0; i < setting->nowOffsetList->length(); i++)
    {
        if(setting->nowOffsetList->at(i).SN == ui->lineEdit_Tooling->text())
        {
            messageBox.setText("tooling SN conflict.");
            messageBox.show();
            qDebug() << "PreScanDialog: tooling SN conflict.";
            ui->lineEdit_Tooling->clear();
            ui->lineEdit_Tooling->setFocus();
            return;
        }
    }

    int nowBaseNum = ui->lineEdit_Base->text().section('-',2,2).toInt();

    //檢查此base位置是否已經有擺放東西
    if(!setting->nowOffsetList->at(nowBaseNum-1).ID.isEmpty())
    {
        messageBox.setText("Base conflict.");
        messageBox.show();
        qDebug() << "PreScanDialog: Base conflict.";
        ui->lineEdit_Base->clear();
        ui->lineEdit_Base->setFocus();
        return;
    }   

    //將檢查pass的offset加入新的offsetList
    setting->nowOffsetList->replace(nowBaseNum-1, offset);

    //將讀取到的入料區對應位置存入
    setting->nowElectorstaticBoxList->replace(nowBaseNum-1, ui->lineEdit_ElecBox->text().section('-',1,1).toInt());

    //if 判斷檢查通過
    ui->groupBox->setEnabled(false);

    emit finished();

    for(int i = 0; i < setting->nowOffsetList->length(); i++)
    {
        if(setting->nowOffsetList->at(i).SN.isEmpty())
        {
            return;
        }
    }

    emit allComplete();
}

void PreScanGroupBox::on_lineEdit_Tooling_returnPressed()
{
    ui->lineEdit_Tooling->setText(ui->lineEdit_Tooling->text().toUpper());

    QRegExp rx("^FX-[A-Z0-9]*-[0-9]{3,3}$");
    if(!rx.exactMatch(ui->lineEdit_Tooling->text()))
    {
        messageBox.setText("Tooling SN syntax error!");
        messageBox.show();
        qDebug() << "PreScanDialog: Tooling SN syntax error!";
        ui->lineEdit_Tooling->clear();
        return;
    }

    keyInFinished();
}

void PreScanGroupBox::on_lineEdit_Base_returnPressed()
{
    ui->lineEdit_Base->setText(ui->lineEdit_Base->text().toUpper());

    QRegExp rx("^TABLE-BASE-[0-9]{1,3}$");

    if(!rx.exactMatch(ui->lineEdit_Base->text()))
    {
        messageBox.setText("Base SN syntax error!");
        messageBox.show();
        qDebug() << "PreScanDialog: Base SN syntax error!";
        ui->lineEdit_Base->clear();
        return;
    }
    keyInFinished();
}

void PreScanGroupBox::on_lineEdit_ModuleBase_returnPressed()
{
    ui->lineEdit_ModuleBase->setText(ui->lineEdit_ModuleBase->text().toUpper());

    QRegExp rx("^MBASE-[0-9]{1,3}$");

    if(!rx.exactMatch(ui->lineEdit_ModuleBase->text()))
    {
        messageBox.setText("ModuleBase SN syntax error!");
        messageBox.show();
        qDebug() << "PreScanDialog: ModuleBase SN syntax error!";
        ui->lineEdit_ModuleBase->clear();
        return;
    }
    keyInFinished();
}

void PreScanGroupBox::on_lineEdit_Module_returnPressed()
{
    ui->lineEdit_Module->setText(ui->lineEdit_Module->text().toUpper());

    QRegExp rx("^FX-[A-Z0-9]*-[0-9]{3,3}$");

    if(!rx.exactMatch(ui->lineEdit_Module->text()))
    {
        messageBox.setText("Module SN syntax error!");
        messageBox.show();
        qDebug() << "PreScanDialog: Module SN syntax error!";
        ui->lineEdit_Module->clear();
        return;
    }
    keyInFinished();
}

void PreScanGroupBox::on_lineEdit_ElecBox_returnPressed()
{
    ui->lineEdit_ElecBox->setText(ui->lineEdit_ElecBox->text().toUpper());

    QRegExp rx("^LOAD-00[0-2]$");

    if(!rx.exactMatch(ui->lineEdit_ElecBox->text()))
    {
        messageBox.setText("Electrostatic box syntax error!");
        messageBox.show();
        qDebug() << "PreScanDialog: Electrostatic box syntax error!";
        ui->lineEdit_ElecBox->clear();
        return;
    }
    keyInFinished();
}

void PreScanGroupBox::StartEdit()
{
    ui->lineEdit_Tooling->setFocus();
}

void PreScanGroupBox::ReKeyLineEdit(const LineEdit &_lineEdit)
{
    ui->groupBox->setEnabled(true);

    switch (_lineEdit) {
    case TOOLING:
        ui->lineEdit_Tooling->clear();
        ui->lineEdit_Tooling->setFocus();
        break;
    case BASE:
        ui->lineEdit_Base->clear();
        ui->lineEdit_Base->setFocus();
        break;
    case MODULE_BASE:
        ui->lineEdit_ModuleBase->clear();
        ui->lineEdit_ModuleBase->setFocus();
        break;
    case MODULE:
        ui->lineEdit_Module->clear();
        ui->lineEdit_Module->setFocus();
        break;
    default:
        qCritical() << "Error: PreScanGroupBox::ReKeyLineEdit()\nCase exception error.";
        break;
    }
}


