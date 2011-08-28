#ifndef BRIDGE_H
#define BRIDGE_H

/*
 * Bridge is the actual controller that bridges Serial <-> MIDI. Create it by passing the
 * the serial & MIDI ports to use (or NULL if that part is not used.)
 *
 * The Bridge manages its own internal state, and can be deleted when you stop bridging and/or
 * recreated with new settings.
 */

#include <QObject>
#include "RtMidi.h"
#include "QRtMidiIn.h"
#include "qextserialport/qextserialport.h"

const QString TEXT_NOT_CONNECTED = "(Not Connected)";
const QString TEXT_NEW_PORT = "(Create new port)";

#define NAME_MIDI_IN "MIDI->Serial"
#define NAME_MIDI_OUT "Serial->MIDI"

class Bridge : public QObject
{
    Q_OBJECT
public:
    explicit Bridge(QObject *parent);
    void attach(QString serialName, PortSettings &serialSettings, int midiInPort, int midiOutPort);

    // Destroying an existing Bridge will cleanup state & release all ports
    ~Bridge();

private:
    QString bridgeName();

signals:
    // Signals to push user status messages
    void displayMessage(QString message);
    void debugMessage(QString message);

    // Signals to trigger UI "blinkenlight" updates
    void midiReceived();
    void midiSent();
    void serialTraffic();
private slots:
    void onMidiIn(double timeStamp, QByteArray message);
    void onSerialAvailable();
private:
    int tryMatchSerial(QByteArray &buf);
    int scanSerialNoise(QByteArray &buf);
    int scanSerialDebugMessage(QByteArray &buf);
    int scanSerialMidiMessage(QByteArray &buf);

    QByteArray serialBuf;
    QRtMidiIn *midiIn;
    RtMidiOut *midiOut;
    int midiInPort;
    int midiOutPort;
    QextSerialPort *serial;
};

#endif // BRIDGE_H
