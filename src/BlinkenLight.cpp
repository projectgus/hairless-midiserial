#include "BlinkenLight.h"
#include <QPixmapCache>

const int DEFAULT_LED_BLINKTIME= 75; // ms

const QString RES_LED_ON = ":/images/images/led-on.png";
const QString RES_LED_OFF = ":/images/images/led-off.png";

BlinkenLight::BlinkenLight(QWidget *parent) :
    QLabel(parent),
    on(false),
    timer()
{
    timer.setSingleShot(true);
    timer.setInterval(DEFAULT_LED_BLINKTIME);
    connect(&timer, SIGNAL(timeout()), this, SLOT(blinkOff()));
}

static QPixmap getCachedPixmap(const QString name)
{
    QPixmap res;
    if(!QPixmapCache::find(name, &res)) {
        res = QPixmap::fromImage(QImage(name));
        QPixmapCache::insert(name, res);
    }
    return res;
}

void BlinkenLight::blinkOn()
{
    if(on) return;
    timer.start();
    this->setPixmap(getCachedPixmap(RES_LED_ON));
    on = true;
}

void BlinkenLight::blinkOff()
{
    if(!on) return;
    this->setPixmap(getCachedPixmap(RES_LED_OFF));
    on = false;
}
