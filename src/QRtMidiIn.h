#ifndef QRTMIDIIN_H
#define QRTMIDIIN_H

/*
 * Very thin QT wrapper for the cross-platform RtMidiIn class.
 *
 * Exposes the "new MIDI message" callback as a QT signal, which can be sent to a QT slot.
 */
#include "RtMidi.h"
#include <QObject>
#include <QByteArray>
#include <stdint.h>

class QRtMidiIn : public QObject, public RtMidiIn {
    Q_OBJECT

public:
    QRtMidiIn(const std::string clientName = std::string( "QRtMidi Input Client") );
    void midiCallback(double timeStamp, std::vector<unsigned char> *message);
    virtual ~QRtMidiIn ( void ) throw() { }
signals:
    void messageReceived(double timeStamp, QByteArray message);
};

#endif // QRTMIDIIN_H
