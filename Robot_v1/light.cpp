#include "light.h"
#include <QTimer>
#include <QDebug>

Light::Light(QWidget *parent) : QLabel(parent)
{
    setScaledContents(true);
    setPixmap(QPixmap(":/lights/IMG_Grey.png"));

    flashTimer = new QTimer(this);
    flashTimer->setInterval(500);
    flashOnOff = true;
}

void Light::setFlashTimer(QTimer *_timer)
{
    flashTimer = _timer;
}

void Light::setLight(Color _color, bool _flash)
{
    disconnect(flashTimer, SIGNAL(timeout()), this, SLOT(flashRedTimeout()));
    disconnect(flashTimer, SIGNAL(timeout()), this, SLOT(flashGreenTimeout()));
    disconnect(flashTimer, SIGNAL(timeout()), this, SLOT(flashYellowTimeout()));

    if(_flash)
    {
        switch(_color)
        {
        case GREEN:
            flashOnOff = true;
            connect(flashTimer, SIGNAL(timeout()), this, SLOT(flashGreenTimeout()));
            flashTimer->start();
            break;
        case YELLOW:
            flashOnOff = true;
            connect(flashTimer, SIGNAL(timeout()), this, SLOT(flashYellowTimeout()));
            flashTimer->start();
            break;
        case RED:
            flashOnOff = true;
            connect(flashTimer, SIGNAL(timeout()), this, SLOT(flashRedTimeout()));
            flashTimer->start();
            break;
        default:
            qDebug() << "Error: Light::setLight";
            break;
        }
    }
    else
    {
        flashTimer->stop();
        switch(_color)
        {
        case GREY:
            setPixmap(QPixmap(":/lights/IMG_Grey.png"));
            break;
        case GREEN:
            setPixmap(QPixmap(":/lights/IMG_Green.png"));
            break;
        case YELLOW:
            setPixmap(QPixmap(":/lights/IMG_Yellow.png"));
            break;
        case RED:
            setPixmap(QPixmap(":/lights/IMG_Red.png"));
            break;
        default:
            qDebug() << "Error: Light::setLight";
            break;
        }
    }
}

void Light::flashYellowTimeout()
{
    //Flashing by switch yellow and grey
    if(flashOnOff)
    {
        setPixmap(QPixmap(":/lights/IMG_Grey.png"));
    }
    else{
        setPixmap(QPixmap(":/lights/IMG_Yellow.png"));
    }
    flashOnOff = !flashOnOff;
}

void Light::flashGreenTimeout()
{
    //Flashing by switch yellow and grey
    if(flashOnOff)
    {
        setPixmap(QPixmap(":/lights/IMG_Grey.png"));
    }
    else{
        setPixmap(QPixmap(":/lights/IMG_Green.png"));
    }
    flashOnOff = !flashOnOff;
}

void Light::flashRedTimeout()
{
    //Flashing by switch yellow and grey
    if(flashOnOff)
    {
        setPixmap(QPixmap(":/lights/IMG_Grey.png"));
    }
    else{
        setPixmap(QPixmap(":/lights/IMG_Red.png"));
    }
    flashOnOff = !flashOnOff;
}

void Light::OnlineLighting()
{
    setLight(GREEN, false);
}

void Light::OfflineLighting()
{
    setLight(GREY, false);
}

void Light::excutingLighting()
{
    setLight(YELLOW, true);
}

void Light::ErrorLighting()
{
    setLight(RED, false);
}
