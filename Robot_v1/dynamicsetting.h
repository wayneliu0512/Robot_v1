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
//    對應靜電盒位置列表
    QVector<int> *electrostaticBoxList;
//    更新Base
    void updateBaseSetting();
//    回到讀取Base座標系的列表
    void backToOriginalOffset();
//    調整機箱3的base 參數
    void adjustTooling3Offset();

private:

//    讀取base_setting ini
    void readBaseSetting();

//    讀取module_setting ini
    void readModuleSetting();

    int toolingQuantity;

};

#endif // DYNAMICSETTING_H
