#ifndef DYNAMICSETTING_H
#define DYNAMICSETTING_H
#include <QObject>

/*動態設定系統:
    讀取base 參數檔, 調整機箱3的base 參數
    更新base 參數檔
*/

class Base;
class DynamicSetting : public QObject
{
    Q_OBJECT
public:
    explicit DynamicSetting(QObject *parent = 0, const int &_toolingQuantity = 0);

    ~DynamicSetting();
//    讀取Base座標系的列表 [0]=第一個機台的offset值, [1]=第二個機台的offset值
    QVector<Base> *offsetList;
//    現在Base座標系的列表, 並更新至offsetList
    QVector<Base> *nowOffsetList;
//    讀取對應入料區位置列表
    QVector<int> *electrostaticBoxList;
//    現在對應入料區位置列表
    QVector<int> *nowElectrostaticBoxList;
//    更新Base,  更新對應入料區位置列表
    void updateSetting();
//    回到讀取Base座標系的列表, 回到讀取的對應入料區列表
    void backToOriginalSetting();
//    調整機箱3的base 參數
    void adjustTooling3Offset();

private:

//    讀取base_setting ini
    void readBaseSetting();
//    讀取module_setting ini
    void readModuleSetting();
//    分類入料區有幾種機種, 如果全部都是入料區1, 則把所有入料區切換成3
    void classifyElecBoxList();
//    機箱數量
    int toolingQuantity;

};

#endif // DYNAMICSETTING_H
