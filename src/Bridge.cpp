#include "Bridge.h"
#include "qextserialport/qextserialenumerator.h"
#include <QList>
#include <QVector>
#include <stdint.h>

Bridge::Bridge(QObject *parent, QString *serialName, PortSettings *serialSettings, int *midiInPort, int *midiOutPort) :
    QObject(parent)
{
    if(serialName && serialSettings) {
        emit displayMessage("Opening serial port" + *serialName);
        this->serial = new QextSerialPort(*serialName, *serialSettings);
        connect(this->serial, SIGNAL(readyRead()), this, SLOT(onSerialAvailable()));
    }
    if(midiInPort) {
        emit displayMessage("Opening MIDI In port #" + *midiInPort);
        this->midiInPort = midiInPort;
        this->midiIn = new QRtMidiIn("Serial->MIDI Port");
        this->midiIn->openPort(*midiInPort);
        connect(this->midiIn, SIGNAL(messageReceived(double,QVector<uint8_t>)), this, SLOT(onMidiIn(double,QVector<uint8_t>)));
        // todo: catch exceptions
    }
    if(midiOutPort) {
        emit displayMessage("Opening MIDI Out port #" + *midiOutPort);
        this->midiOutPort = midiOutPort;
        this->midiOut = new RtMidiOut("MIDI->Serial Port");
        this->midiOut->openPort(*midiOutPort);
            // todo: catch exceptions
    }
}

Bridge::~Bridge()
{
    emit displayMessage("Closing bridge " + bridgeName());
    delete this->midiIn;
    delete this->midiOut;
    delete this->serial;
}

QString Bridge::bridgeName()
{
    return QString("(Bridge %1%2%3)")
                     .arg(midiIn ? QString("Port #%d -> ").arg(*midiInPort) : "")
                     .arg(serial ? serial->portName() : "NO SERIAL")
                     .arg(midiOut ? QString(" -> Port #%d").arg(*midiOutPort) : "")
                     ;
}

void Bridge::onMidiIn(double timeStamp, QByteArray message)
{
    emit debugMessage(QString("MIDI In @ %1: %2").arg(timeStamp).arg(QString(message.toHex())));
    // TODO: pad MIDI message where applicable
    if(serial && serial->isOpen()) {
        serial->write(message);
    }
}

void Bridge::onSerialAvailable()
{
    QByteArray data = this->serial->readAll();
    emit debugMessage(QString("Serial In: %1").arg(QString(data.toHex())));
    // TODO: accumulate serial bytes, check for debug output, etc.
    if(midiOut) {
        std::vector<uint8_t> message = std::vector<uint8_t>(data.begin(), data.end());
        midiOut->sendMessage(&message);
    }
}

