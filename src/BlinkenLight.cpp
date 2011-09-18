#include "BlinkenLight.h"

const int DEFAULT_LED_BLINKTIME= 75; // ms

const QString RES_LED_ON = ":/images/images/led-on.png";
const QString RES_LED_OFF = ":/images/images/led-off.png";

static QPixmap pxLedOn = NULL;
static QPixmap pxLedOff = NULL;

BlinkenLight::BlinkenLight(QWidget *parent) :
    QLabel(parent),
    on(false),
    timer()
{
    if(!pxLedOn) {
        pxLedOn = QPixmap::fromImage(QImage(RES_LED_ON));
        pxLedOff = QPixmap::fromImage(QImage(RES_LED_OFF));
    }
    timer.setSingleShot(true);
    timer.setInterval(DEFAULT_LED_BLINKTIME);
    connect(&timer, SIGNAL(timeout()), this, SLOT(blinkOff()));
}

void BlinkenLight::blinkOn()
{
    if(on) return;
    timer.start();
    this->setPixmap(pxLedOn);
    on = true;
}

void BlinkenLight::blinkOff()
{
    if(!on) return;
    this->setPixmap(pxLedOff);
    on = false;
}
