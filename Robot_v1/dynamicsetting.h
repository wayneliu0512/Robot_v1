#ifndef DYNAMICSETTING_H
#define DYNAMICSETTING_H

#include <QObject>

class Base;

class DynamicSetting : public QObject
{
    Q_OBJECT
public:
    explicit DynamicSetting(QObject *parent = 0, const int &_toolingQuantity = 0);

    ~DynamicSetting();
    //儲存Base座標系的列表 [0]=第一個機台的offset值, [1]=第二個機台的offset值
    QVector<Base> *offsetList;
    QVector<Base> *nowOffsetList;
    //更新Base
    void updateBaseSetting();
    void backToOriginalOffset();
    void adjustTooling3Offset();

private:

    //讀取base_setting ini
    void readBaseSetting();

    //讀取module_setting ini
    void readModuleSetting();

    int toolingQuantity;

};

#endif // DYNAMICSETTING_H
