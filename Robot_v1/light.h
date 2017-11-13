#ifndef LIGHT_H
#define LIGHT_H
#include <QLabel>

/* 自訂 Widget -> 燈號裝置：
   紅燈, 綠燈, 黃燈, 灰燈
   閃爍/無閃爍
*/

class Light : public QLabel
{
    Q_OBJECT
public:
    enum Color{ GREY, GREEN, YELLOW, RED };

    explicit Light(QWidget *parent = 0);
//    設定閃爍間隔時間
    void setFlashTimer(QTimer *_timer);
//    顏色, 有無閃爍 介面
    void setLight(Color _color, bool _flash);
signals:

public slots:
    void OnlineLighting();
    void OfflineLighting();
    void ErrorLighting();
    void excutingLighting();
private slots:
    void flashGreenTimeout();
    void flashYellowTimeout();
    void flashRedTimeout();

private:
    QTimer *flashTimer;

    bool flashOnOff;
};

#endif // LIGHT_H
