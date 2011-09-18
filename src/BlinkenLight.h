#ifndef BLINKENLIGHT_H
#define BLINKENLIGHT_H

#include <QObject>
#include <QLabel>
#include <QTimer>

/*
 * A custom QLabel that looks like a blinking LED.
 * Handles blinking off after a specified time, all by itself.
 */
class BlinkenLight : public QLabel
{
    Q_OBJECT
public:
    explicit BlinkenLight(QWidget *parent = 0);
    void setBlinkTime(int blinkTime) { this->timer.setInterval(blinkTime); }
    ~BlinkenLight() { }
private:
    bool on;
    QTimer timer;
public slots:
    void blinkOn();
private slots:
    void blinkOff();
};

#endif // BLINKENLIGHT_H
