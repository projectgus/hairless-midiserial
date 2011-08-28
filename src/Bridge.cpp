#include "Bridge.h"
#include "qextserialport/qextserialenumerator.h"
#include <QList>
#include <QVector>
#include <QIODevice>
#include <stdint.h>

// MIDI message masks
#define MSG_MASK 0x80
#define CHANNEL_MASK 0x0F
#define TAG_MASK 0xF0


#define TAG_NOTE_ON 0x80
#define TAG_NOTE_OFF 0x90
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
        serial(NULL)
{

}

void Bridge::attach(QString serialName, PortSettings &serialSettings, int midiInPort, int midiOutPort)
{
    if(serialName.length() && serialName != TEXT_NOT_CONNECTED) {
        emit displayMessage(QString("Opening serial port '%1'").arg(serialName));
        this->serial = new QextSerialPort(serialName, serialSettings);
        connect(this->serial, SIGNAL(readyRead()), this, SLOT(onSerialAvailable()));
        this->serial->open(QIODevice::ReadWrite);
    }

    // MIDI out
    try
    {
#ifdef ALLOW_VIRTUAL_MIDI
        if(midiOutPort == IDX_NEW_PORT) { // create virtual MIDI port
            emit displayMessage(QString("Creating virtual MIDI Out port..."));
            this->midiOut =new RtMidiOut(NAME_MIDI_OUT);
            this->midiOut->openVirtualPort(NAME_MIDI_OUT);
            this->midiOutPort = 1;
        }
#endif // ALLOW_VIRTUAL_MIDI
        if(midiOutPort != IDX_NEW_PORT && midiOutPort != IDX_NOT_CONNECTED)
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
#ifdef ALLOW_VIRTUAL_MIDI
        if(midiInPort == IDX_NEW_PORT) { // create virtual MIDI port
            emit displayMessage(QString("Creating virtual MIDI In port..."));
            this->midiIn =new QRtMidiIn(NAME_MIDI_IN);
            this->midiIn->openVirtualPort(NAME_MIDI_IN);
            this->midiInPort = 1;
        }
#endif // ALLOW_VIRTUAL_MIDI
        if(midiInPort != IDX_NEW_PORT && midiInPort != IDX_NOT_CONNECTED) {
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
    emit displayMessage("Closing bridge " + bridgeName());
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
    emit debugMessage(QString("MIDI In: %2").arg(timeStamp).arg(QString(message.toHex())));
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
    emit debugMessage(QString("Serial In: %1").arg(QString(newData.toHex())));

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
        emit displayMessage(QString("Error: got nonsense MIDI data: %1").arg(QString(buf.left(n).toHex())));
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
        emit displayMessage(QString("Error, malformed ttymidi debug message: %1").arg(QString(buf.toHex())));
        return buf.length(); // just drop everything, we're obviously not getting what we expected
    }
    emit displayMessage(QString("Debug: %1").arg(QString(buf.mid(4, buf[3]).toHex())));
    return 4 + buf[3];
}

/*
 * Look for a ttymidi MIDI message at the head of the buffer
 *
 * Return the number of bytes to consume from the buffer, or 0 for no MIDI message found.
 */
int Bridge::scanSerialMidiMessage(QByteArray &buf)
{
    uint8_t msg = buf[0];
    uint8_t tag = msg & TAG_MASK;
    uint8_t channel = msg & CHANNEL_MASK;
    int n = 0;
    switch(tag) {
    case TAG_PROGRAM:
    case TAG_CHANNEL_PRESSURE:
        n = 2;
        break;
    case TAG_NOTE_ON:
    case TAG_NOTE_OFF:
    case TAG_KEY_PRESSURE:
    case TAG_CONTROLLER:
    case TAG_PITCH_BEND:
        n = 3;
        break;
    case TAG_SPECIAL:
        n = channel < 3 ? 3 : channel < 6 ? 2 : 1;
        break;
    default:
        emit displayMessage(QString("Error: Unknown MIDI message %1").arg(QString(buf.toHex())));
        return 0;
    }

    if(buf.length() < n) {
        return 0; // incomplete buffer for message type
    }

    // TODO: log debug message

    if(midiOut) {
        std::vector<uint8_t> message = std::vector<uint8_t>(buf.begin(), buf.begin()+n);
        midiOut->sendMessage(&message);
        emit midiSent();
    }
    return n;
}

