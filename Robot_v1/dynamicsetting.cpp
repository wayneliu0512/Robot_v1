#include "dynamicsetting.h"
#include "base.h"
#include <QSettings>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
DynamicSetting::DynamicSetting(QObject *parent, const int &_toolingQuantity) :
    QObject(parent), toolingQuantity(_toolingQuantity)
{
    offsetList = new QVector<Base>(toolingQuantity);
    nowOffsetList = new QVector<Base>(toolingQuantity);
    electrostaticBoxList = new QVector<int>(toolingQuantity);

    readBaseSetting();
}

DynamicSetting::~DynamicSetting()
{
    delete offsetList;
    delete nowOffsetList;
}

void DynamicSetting::readBaseSetting()
{
    offsetList->clear();
    offsetList->resize(toolingQuantity);

    QSettings baseIni("Base_Setting.ini", QSettings::IniFormat);

    QList<int> matchBase;

    for(int i = 0; i < toolingQuantity; i++)
    {
        matchBase.append(baseIni.value("BaseMatch/Base" + QString::number(i+1)).toInt());
    }

    QStringList baseListIni = baseIni.childGroups();

    for(int i = 0; i < toolingQuantity; i++)
    {
        baseIni.beginGroup(baseListIni.at(matchBase.at(i)));
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

void DynamicSetting::backToOriginalOffset()
{
    *nowOffsetList = *offsetList;
}

void DynamicSetting::adjustTooling3Offset()
{
    Base base = nowOffsetList->at(2);
    base.Y = QString::number(base.Y.toDouble() + base.width.toDouble());

    nowOffsetList->replace(2, base);
}

void DynamicSetting::updateBaseSetting()
{
    QSettings baseIni("Base_Setting.ini", QSettings::IniFormat);

    for(int i = 0; i < toolingQuantity; i++)
    {
        baseIni.setValue("BaseMatch/Base" + QString::number(i+1) , nowOffsetList->at(i).ID);
    }

    readBaseSetting();
}
