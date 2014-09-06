#include "QRtMidiIn.h"

static void midiCallbackOuter(double timeStamp, std::vector<unsigned char> *message, void *userData)
{
    QRtMidiIn *midi =(QRtMidiIn *)userData;
    midi->midiCallback(timeStamp, message);
}

QRtMidiIn::QRtMidiIn(const std::string clientName) :
            QObject(0),
            RtMidiIn(RtMidi::UNSPECIFIED, clientName)
{
    this->ignoreTypes(false,false,false); // we want MIDI SysEx, time & sense messages
    this->setCallback(midiCallbackOuter, this);
}

void QRtMidiIn::midiCallback(double timeStamp, std::vector<unsigned char> *message)
{
    QByteArray messageCopy = QByteArray((const char *)message->data(), message->size());
    emit messageReceived(timeStamp, messageCopy);
}

