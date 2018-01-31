#include "dynamicsetting.h"
#include "base.h"
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"

DynamicSetting::DynamicSetting(QObject *parent, const int &_toolingQuantity) :
    QObject(parent), toolingQuantity(_toolingQuantity)
{    
    offsetList = new QVector<Base>(toolingQuantity);
    nowOffsetList = new QVector<Base>(toolingQuantity);
    electrostaticBoxList = new QVector<int>(toolingQuantity);
    nowElectrostaticBoxList = new QVector<int>(toolingQuantity);

    readBaseSetting();
}

DynamicSetting::~DynamicSetting()
{
    delete offsetList;
    delete nowOffsetList;
    delete electrostaticBoxList;
    delete nowElectrostaticBoxList;
}

void DynamicSetting::readBaseSetting()
{
    offsetList->clear();
    offsetList->resize(toolingQuantity);
    electrostaticBoxList->clear();
    electrostaticBoxList->resize(toolingQuantity);

#if defined(Q_OS_MAC)
    QSettings baseIni("/Users/wayneliu/Documents/Qt/build-Robot_v1-Desktop_Qt_5_8_0_clang_64bit-Release/Base_Setting.ini", QSettings::IniFormat);
#elif defined(Q_OS_WIN)
    QSettings baseIni("Base_Setting.ini", QSettings::IniFormat);
#endif

//    讀取Base對應的入料區位置
    for(int i = 0; i < toolingQuantity; ++i)
    {
        electrostaticBoxList->replace(i, baseIni.value("ElectrostaticBoxMatch/Base" + QString::number(i+1)).toInt());
    }

//    讀取Base對應的機箱Offset
    QList<int> matchBase;
    for(int i = 0; i < toolingQuantity; i++)
    {
        matchBase.append(baseIni.value("BaseMatch/Base" + QString::number(i+1)).toInt());
    }

    QStringList baseListIni = baseIni.childGroups();

    for(int i = 0; i < toolingQuantity; i++)
    {
        baseIni.beginGroup(baseListIni.at(matchBase.at(i)+1));
        QStringList baseKey = baseIni.childKeys();

        Base base;
        base.ID = QString::number(matchBase.at(i));
        for(int c = 0; c < baseKey.length(); c++)
        {
            if(baseKey.at(c) == "X")
            {
                base.X = baseIni.value("X").toString();
            }else if(baseKey.at(c) == "Y")
            {
                base.Y = baseIni.value("Y").toString();
            }else if(baseKey.at(c) == "Z")
            {
                base.Z = baseIni.value("Z").toString();
            }else if(baseKey.at(c) == "A")
            {
                base.A = baseIni.value("A").toString();
            }else if(baseKey.at(c) == "B")
            {
                base.B = baseIni.value("B").toString();
            }else if(baseKey.at(c) == "C")
            {
                base.C = baseIni.value("C").toString();
            }else if(baseKey.at(c) == "SN")
            {
                base.SN = baseIni.value("SN").toString();
                base.type = base.SN.section('-', 1, 1);
            }else if(baseKey.at(c) == "Width")
            {
                base.width = baseIni.value("Width").toString();
            }
        }
        offsetList->replace(i, base);
        baseIni.endGroup();
    }
}

void DynamicSetting::readModuleSetting()
{
    QSettings moduleSetting("Module_Setting.ini", QSettings::IniFormat);
}

//調整Tooling3 位置的偏移量(目前沒有用到)
void DynamicSetting::adjustTooling3Offset()
{
    Base base = nowOffsetList->at(2);
    base.Y = QString::number(base.Y.toDouble() + base.width.toDouble());

    nowOffsetList->replace(2, base);
}

//辨別目前是單機種 or 雙機種
void DynamicSetting::classifyElecBoxList()
{
    for(int i = 0; i < toolingQuantity; ++i)
    {
        if(nowElectrostaticBoxList->at(i) != 1)
        {
            MainWindow::trayMode = MainWindow::TWO_TYPES;
            qDebug() << "ElecBoxList: two type mode";
            return;
        }
    }
    for(int i = 0; i < toolingQuantity; ++i)
    {
        nowElectrostaticBoxList->replace(i, 3);
    }
    MainWindow::trayMode = MainWindow::ONE_TYPE;
    qDebug() << "ElecBoxList: one type mode";
}

//將offset 還原設定
void DynamicSetting::backToOriginalSetting()
{
    *nowOffsetList = *offsetList;
    *nowElectrostaticBoxList = *electrostaticBoxList;

    classifyElecBoxList();
}

//更新Base_Setting.ini
void DynamicSetting::updateSetting()
{    
    QSettings baseIni("Base_Setting.ini", QSettings::IniFormat);
    for(int i = 0; i < toolingQuantity; i++)
    {
        baseIni.setValue("BaseMatch/Base" + QString::number(i+1) , nowOffsetList->at(i).ID);
        baseIni.setValue("ElectrostaticBoxMatch/Base" + QString::number(i+1), nowElectrostaticBoxList->at(i));
    }

    classifyElecBoxList();
//    readBaseSetting();
}
