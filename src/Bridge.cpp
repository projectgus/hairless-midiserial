#include "Bridge.h"
#include "qextserialport/qextserialenumerator.h"
#include <QList>
#include <QVector>
#include <QIODevice>
#include <stdint.h>
#include "PortLatency.h"

// MIDI message masks
#define MSG_MASK 0x80
#define CHANNEL_MASK 0x0F
#define TAG_MASK 0xF0


#define TAG_NOTE_OFF 0x80
#define TAG_NOTE_ON 0x90
#define TAG_KEY_PRESSURE 0xA0
#define TAG_CONTROLLER 0xB0
#define TAG_PROGRAM 0xC0
#define TAG_CHANNEL_PRESSURE 0xD0
#define TAG_PITCH_BEND 0xE0
#define TAG_SPECIAL 0xF0

#define MSG_DEBUG 0xFF // special ttymidi "debug output" MIDI message tag


Bridge::Bridge(QObject *parent) :
        QObject(parent),
        serialBuf(),
        midiIn(NULL),
        midiOut(NULL),
        serial(NULL),
        latency(NULL),
        attachTime(QTime::currentTime())
{
}

void Bridge::attach(QString serialName, PortSettings &serialSettings, int midiInPort, int midiOutPort)
{
    if(serialName.length() && serialName != TEXT_NOT_CONNECTED) {
        // Latency fixups
        latency = new PortLatency(serialName);
        connect(latency, SIGNAL(debugMessage(QString)), this, SIGNAL(debugMessage(QString)));
        connect(latency, SIGNAL(errorMessage(QString)), this, SIGNAL(displayMessage(QString)));
        latency->fixLatency();

        emit displayMessage(QString("Opening serial port '%1'...").arg(serialName));
        this->serial = new QextSerialPort(serialName, serialSettings);
        connect(this->serial, SIGNAL(readyRead()), this, SLOT(onSerialAvailable()));
        this->serial->open(QIODevice::ReadWrite);
        attachTime = QTime::currentTime();
    }

    // MIDI out
    try
    {
        if(midiOutPort > -1)
        {
            emit displayMessage(QString("Opening MIDI Out port #%1").arg(midiOutPort));
            this->midiOutPort = midiOutPort;
            this->midiOut = new RtMidiOut(NAME_MIDI_OUT);
            this->midiOut->openPort(midiOutPort);
        }
    }
    catch(RtError e)
    {
        displayMessage(QString("Failed to open MIDI out port: %1").arg(QString::fromStdString(e.getMessage())));
    }

    // MIDI in
    try
    {
       if(midiInPort > -1) {
            emit displayMessage(QString("Opening MIDI In port #%1").arg(midiInPort));
            this->midiInPort = midiInPort;
            this->midiIn = new QRtMidiIn(NAME_MIDI_IN);
            this->midiIn->openPort(midiInPort);
            connect(this->midiIn, SIGNAL(messageReceived(double,QByteArray)), this, SLOT(onMidiIn(double,QByteArray)));
        }
     }
    catch(RtError e)
    {
        displayMessage(QString("Failed to open MIDI in port: %1").arg(QString::fromStdString(e.getMessage())));
    }
}

Bridge::~Bridge()
{
    emit displayMessage(applyTimeStamp("Closing bridge " + bridgeName()));
    if(this->latency) {
        this->latency->resetLatency();
    }
    delete this->midiIn;
    delete this->midiOut;
    delete this->serial;
}

QString Bridge::bridgeName()
{
    return QString("(Bridge %1%2%3)")
                     .arg(midiIn ? QString("Port #%1 -> ").arg(midiInPort) : "")
                     .arg(serial ? serial->portName() : "NO SERIAL")
                     .arg(midiOut ? QString(" -> Port #%1").arg(midiOutPort) : "")
                     ;
}

void Bridge::onMidiIn(double timeStamp, QByteArray message)
{
    QString description = describeMIDI(message, 0);
    emit debugMessage(QString("MIDI In: %2").arg(timeStamp).arg(description));
    emit midiReceived();
    while(message.length() < 3) {
        message.append((char)0); // pad short MIDI messages, as per ttymidi
    }
    if(serial && serial->isOpen()) {
        serial->write(message);
        emit serialTraffic();
    }
}

void Bridge::onSerialAvailable()
{
    emit serialTraffic();
    QByteArray newData = this->serial->readAll();

    serialBuf.append(newData);
    while(int n = tryMatchSerial(serialBuf))
    {
        if(n == 0)
            return;
        else
            serialBuf = serialBuf.mid(n);
    }
}

/*
 * Look for any MIDI message we recognise in the serial buffer.
 *
 * Return the number of bytes to consume from the buffer.
 */
int Bridge::tryMatchSerial(QByteArray &buf)
{
    int n;
    if (buf.length() == 0)
        return 0;
    (n = scanSerialNoise(buf) )
        || ( n = scanSerialDebugMessage(buf) )
        || ( n = scanSerialMidiMessage(buf) );
    return n;
}

/*
 * Look for any non-MIDI noise data at head of the buffer.
 *
 * Return the number of bytes of noise to consume from the buffer, or 0 for no noise.
 */
int Bridge::scanSerialNoise(QByteArray &buf)
{
    int n = 0;
    while( n < buf.size() && ( buf[n] & MSG_MASK) == 0 ) { n++; }  // Trim any non-MIDI message flags
    if(n) {
        emit displayMessage(applyTimeStamp(QString("Error: got nonsense MIDI data: %1").arg(QString(buf.left(n).toHex()))));
    }

    return n;
}

/*
 * Look for a ttymidi debug message at the head of the buffer
 *
 * Return the number of bytes to consume from the buffer, or 0 for no debug message.
 */
int Bridge::scanSerialDebugMessage(QByteArray &buf)
{
    uint8_t msg = buf[0];
    if(msg != MSG_DEBUG) {
        return 0; // not a debug message
    }
    if(buf.length() < 4 || buf.length() < buf[3] + 4) {
        return 0; // incomplete
    }
    if(buf[1] || buf[2]) {
        emit displayMessage(applyTimeStamp(QString("Error, malformed ttymidi debug message: %1").arg(QString(buf.toHex()))));
        return buf.length(); // just drop everything, we're obviously not getting what we expected
    }
    emit displayMessage(applyTimeStamp(QString("Serial Says: %1").arg(QString::fromAscii(buf.mid(4, buf[3]).data()))));
    return 4 + buf[3];
}

/*
 * Look for a ttymidi MIDI message at the head of the buffer
 *
 * Return the number of bytes to consume from the buffer, or 0 for no MIDI message found.
 */
int Bridge::scanSerialMidiMessage(QByteArray &buf)
{
    int msg_len = 0;
    QString description = describeMIDI(buf, &msg_len);

    if(buf.length() < msg_len) { // incomplete message, wait for buffer to fill more
        return 0;
    }

    emit debugMessage(applyTimeStamp(QString("Serial In: ").append(description)));

    if(midiOut && msg_len) {
        std::vector<uint8_t> message = std::vector<uint8_t>(buf.begin(), buf.begin()+msg_len);
        midiOut->sendMessage(&message);
        emit midiSent();
    }
    return msg_len;
}

/*
 * Given a buffer containing what is hopefully a MIDI message,
 * return a string describing the MIDI message and (optionally)
 * also let the caller know how long the MIDI message is
 */
QString Bridge::describeMIDI(QByteArray &buf, int *msg_len_out)
{
    uint8_t msg = buf[0];
    uint8_t tag = msg & TAG_MASK;
    uint8_t channel = msg & CHANNEL_MASK;
    int msg_len = 0;
    const char *desc= 0;
    if(msg_len_out) {
        *msg_len_out = 0;
    }

    // Work out what we have

    switch(tag) {
    case TAG_PROGRAM:
        desc = "Program change %1";
        msg_len = 2;
        break;
    case TAG_CHANNEL_PRESSURE:
        desc = "Ch %1: Pressure change";
        msg_len = 2;
        break;
    case TAG_NOTE_ON:
        desc = "Ch %1: Note %2 on  velocity %3";
        msg_len = 3;
        break;
    case TAG_NOTE_OFF:
        desc = "Ch %1: Note %2 off velocity %3";
        msg_len = 3;
        break;
    case TAG_KEY_PRESSURE:
        desc = "Ch %1: Note %2 pressure %3";
        msg_len = 3;
        break;
    case TAG_CONTROLLER:
        desc = "Ch %1: Controller %2 value %3";
        msg_len = 3;
        break;
    case TAG_PITCH_BEND:
        desc = "Ch %1: Pitch bend %2";
        msg_len = 3; // pitch bend is special, see below
        break;
    case TAG_SPECIAL:
        if(channel < 3) {
            desc = "System Message #%1: %2 %3";
            msg_len = 3;
        } else if (channel < 6) {
            desc = "System Message #%1: %2";
            msg_len = 2;
        } else {
            desc = "System Message #%1";
            msg_len = 1;
        }
        break;
    default:
        return QString("Unknown MIDI message %1").arg(QString(buf.toHex()));
    }

    if(msg_len_out) {
        *msg_len_out = msg_len;
    }

    // Check the length

    if(buf.length() < msg_len) {
        return QString("Incomplete MIDI message (length %1, needs %2)").arg(buf.length()).arg(msg_len); // incomplete buffer for message type
    }

    // Format the output

    QString qdesc = QString(desc).arg((int) channel);

    if(tag == TAG_PITCH_BEND) { // special
        return qdesc.arg((int) buf[1] | buf[2]<<7 );
    }

    for(int i = 1; i < msg_len; i++) {
            qdesc = qdesc.arg((int) buf[i]);
    }
    return qdesc;
}

QString Bridge::applyTimeStamp(QString message)
{
    QTime now = QTime::currentTime();
    float msecs = this->attachTime.msecsTo(now) / 1000.0;
    return QString("+%1 - %2").arg(msecs, 4).arg(message);
}

