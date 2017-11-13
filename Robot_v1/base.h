#ifndef BASE_H
#define BASE_H
#include <QString>

/* Base 座標容器*/
class Base
{
public:
    Base();
    QString X, Y, Z, A, B, C;
//  type=機種, ID=對應的機箱位置, width=機箱寬度(用於機箱3轉換base)
    QString type, SN, ID, width;
};

#endif // BASE_H
