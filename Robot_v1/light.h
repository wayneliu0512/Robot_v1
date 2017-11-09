#ifndef LIGHT_H
#define LIGHT_H

#include <QLabel>

class Light : public QLabel
{
    Q_OBJECT
public:
    enum Color{ GREY, GREEN, YELLOW, RED };

    explicit Light(QWidget *parent = 0);

    void setFlashTimer(QTimer *_timer);
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
