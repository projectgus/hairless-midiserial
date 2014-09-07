#include "Bridge.h"
#include "qextserialenumerator.h"
#include <QList>
#include <QVector>
#include <QIODevice>
#include <stdint.h>
#include "PortLatency.h"

// MIDI message masks
const uint8_t STATUS_MASK = 0x80;
const uint8_t CHANNEL_MASK = 0x0F;
const uint8_t  TAG_MASK = 0xF0;

const uint8_t TAG_NOTE_OFF = 0x80;
const uint8_t TAG_NOTE_ON = 0x90;
const uint8_t TAG_KEY_PRESSURE = 0xA0;
const uint8_t TAG_CONTROLLER = 0xB0;
const uint8_t TAG_PROGRAM = 0xC0;
const uint8_t TAG_CHANNEL_PRESSURE = 0xD0;
const uint8_t TAG_PITCH_BEND = 0xE0;
const uint8_t TAG_SPECIAL = 0xF0;

const uint8_t MSG_SYSEX_START = 0xF0;
const uint8_t MSG_SYSEX_END = 0xF7;

const uint8_t MSG_DEBUG = 0xFF; // special ttymidi "debug output" MIDI message tag

inline bool is_voice_msg(uint8_t tag) { return tag >= 0x80 && tag <= 0xEF; };
inline bool is_syscommon_msg(uint8_t tag) { return tag >= 0xF0 && tag <= 0xF7; };
inline bool is_realtime_msg(uint8_t tag) { return tag >= 0xF8 && tag < 0xFF; };


// return the number of data bytes for a given message status byte
#define UNKNOWN_MIDI -2
static int get_data_length(int status) {
    uint8_t channel;
    switch(status & TAG_MASK) {
    case TAG_PROGRAM:
    case TAG_CHANNEL_PRESSURE:
      return 1;
    case TAG_NOTE_ON:
    case TAG_NOTE_OFF:
    case TAG_KEY_PRESSURE:
    case TAG_CONTROLLER:
    case TAG_PITCH_BEND:
      return 2;
    case TAG_SPECIAL:
      if(status == MSG_DEBUG) {
        return 3; // Debug messages go { 0xFF, 0, 0, <len>, Msg } so first we read 3 data bytes to get full length
      }
      if(status == MSG_SYSEX_START) {
        return 65535; // SysEx messages go until we see a SysExEnd
      }
      channel = status & CHANNEL_MASK;
      if(channel < 3) {
        return 2;
      } else if (channel < 6) {
        return 1;
      }
      return 0;
    default:
      // Unknown midi message...?
      return UNKNOWN_MIDI;
    }
}


Bridge::Bridge() :
        QObject(),      
        running_status(0),
        data_expected(0),
        msg_data(),
        midiIn(NULL),
        midiOut(NULL),
        serial(NULL),
        latency(NULL),
        attachTime(QTime::currentTime())
{
}

void Bridge::attach(QString serialName, PortSettings serialSettings, int midiInPort, int midiOutPort, QThread *workerThread)
{
    this->moveToThread(workerThread);
    if(serialName.length() && serialName != TEXT_NOT_CONNECTED) {
        // Latency fixups
        latency = new PortLatency(serialName);
        connect(latency, SIGNAL(debugMessage(QString)), this, SIGNAL(debugMessage(QString)));
        connect(latency, SIGNAL(errorMessage(QString)), this, SIGNAL(displayMessage(QString)));
        latency->fixLatency();

        emit displayMessage(QString("Opening serial port '%1'...").arg(serialName));
        this->serial = new QextSerialPort(serialName, serialSettings);
        connect(this->serial, SIGNAL(readyRead()), this, SLOT(onSerialAvailable()));
        this->serial->open(QIODevice::ReadWrite|QIODevice::Unbuffered);
        attachTime = QTime::currentTime();
        this->serial->moveToThread(workerThread);
    }

    // MIDI out
    try
    {
        if(midiOutPort > -1)
        {
            emit displayMessage(QString("Opening MIDI Out port #%1").arg(midiOutPort));
            this->midiOutPort = midiOutPort;
            this->midiOut = new RtMidiOut(RtMidi::UNSPECIFIED, NAME_MIDI_OUT);
            this->midiOut->openPort(midiOutPort);
        }
    }
    catch(RtMidiError e)
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
            this->midiIn->moveToThread(workerThread);
            this->midiIn->openPort(midiInPort);
            connect(this->midiIn, SIGNAL(messageReceived(double,QByteArray)), this, SLOT(onMidiIn(double,QByteArray)));
        }
     }
    catch(RtMidiError e)
    {
        displayMessage(QString("Failed to open MIDI in port: %1").arg(QString::fromStdString(e.getMessage())));
    }
}

Bridge::~Bridge()
{
    emit displayMessage(applyTimeStamp("Closing MIDI<->Serial bridge..."));
    if(this->latency) {
        this->latency->resetLatency();
    }
    delete this->midiIn;
    delete this->midiOut;
    delete this->serial;
}

void Bridge::onMidiIn(double timeStamp, QByteArray message)
{
    QString description = describeMIDI(message);
    emit debugMessage(applyTimeStamp(QString("MIDI In: %1").arg(description)));
    emit midiReceived();
    if(serial && serial->isOpen()) {
        serial->write(message);
        emit serialTraffic();
    }
}

void Bridge::onSerialAvailable()
{
    emit serialTraffic();
    QByteArray data = this->serial->readAll();
    foreach(uint8_t next, data) {
      if (next & STATUS_MASK)
        this->onStatusByte(next);
      else
        this->onDataByte(next);
      if(this->data_expected == 0)
        sendMidiMessage();
    }
}

void Bridge::onStatusByte(uint8_t byte) {
  if(byte == MSG_SYSEX_END && bufferStartsWith(MSG_SYSEX_START)) {
    this->msg_data.append(byte); // bookends of a complete SysEx message
    sendMidiMessage();
    return;
  }

  if(this->data_expected > 0) {
    emit displayMessage(applyTimeStamp(QString("Warning: got a status byte when we were expecting %1 more data bytes, sending possibly incomplete MIDI message 0x%2").arg(this->data_expected).arg((uint8_t)this->msg_data[0], 0, 16)));
    sendMidiMessage();
  }

  if(is_voice_msg(byte))
    this->running_status = byte;
  if(is_syscommon_msg(byte))
    this->running_status = 0;

  this->data_expected = get_data_length(byte);
  if(this->data_expected == UNKNOWN_MIDI) {
      emit displayMessage(applyTimeStamp(QString("Warning: got unexpected status byte %1").arg((uint8_t)byte,0,16)));
      this->data_expected = 0;
  }
  this->msg_data.clear();
  this->msg_data.append(byte);
}

void Bridge::onDataByte(uint8_t byte)
{
  if(this->data_expected == 0 && this->running_status != 0) {
    onStatusByte(this->running_status);
  }
  if(this->data_expected == 0) { // checking again just in in case running status failed to update us to expect data
    emit displayMessage(applyTimeStamp(QString("Error: got unexpected data byte 0x%1.").arg((uint8_t)byte,0,16)));
    return;
  }

  this->msg_data.append(byte);
  this->data_expected--;

  if( bufferStartsWith(MSG_DEBUG)
      && this->data_expected == 0
      && this->msg_data.length() == 4) { // we've read the length of the debug message
    this->data_expected += this->msg_data[3]; // add the message length
  }
}

void Bridge::sendMidiMessage() {
  if(msg_data.length() == 0)
    return;
  if(bufferStartsWith(MSG_DEBUG)) {
      QString debug_msg = QString::fromAscii(msg_data.mid(4, msg_data[3]).data());
      emit displayMessage(applyTimeStamp(QString("Serial Says: %1").arg(debug_msg)));
  } else {
      emit debugMessage(applyTimeStamp(QString("Serial In: %1").arg(describeMIDI(msg_data))));
      if(midiOut) {
        std::vector<uint8_t> message = std::vector<uint8_t>(msg_data.begin(), msg_data.end());
        midiOut->sendMessage(&message);
        emit midiSent();
      }
  }
  msg_data.clear();
  data_expected = 0;
}

/*
 * Given a buffer containing what is hopefully a MIDI message,
 * return a string describing the MIDI message.
 */
QString Bridge::describeMIDI(QByteArray &buf)
{
    uint8_t msg = buf[0];
    uint8_t tag = msg & TAG_MASK;
    uint8_t channel = msg & CHANNEL_MASK;
    const char *desc= 0;

    // Work out what we have

    switch(tag) {
    case TAG_PROGRAM:
        desc = "Ch %1: Program change %2";
        break;
    case TAG_CHANNEL_PRESSURE:
        desc = "Ch %1: Pressure change %2";
        break;
    case TAG_NOTE_ON:
        desc = "Ch %1: Note %2 on  velocity %3";
        break;
    case TAG_NOTE_OFF:
        desc = "Ch %1: Note %2 off velocity %3";
        break;
    case TAG_KEY_PRESSURE:
        desc = "Ch %1: Note %2 pressure %3";
        break;
    case TAG_CONTROLLER:
        desc = "Ch %1: Controller %2 value %3";
        break;
    case TAG_PITCH_BEND:
        desc = "Ch %1: Pitch bend %2";
        break;
    case TAG_SPECIAL:
        if(msg == MSG_SYSEX_START) {
            QString res = "SysEx Message: ";
            for(int i = 1; i < buf.length(); i++) {
                uint8_t val = buf[i];
                if(val == MSG_SYSEX_END)
                    break;
                res.append(QString("0x%1 ").arg(val,0,16));
            }
            return res;
        }
        if(channel < 3) {
            desc = "System Message #%1: %2 %3";
        } else if (channel < 6) {
            desc = "System Message #%1: %2";
        } else {
            desc = "System Message #%1";
        }
        break;
    default:
        return QString("Unknown MIDI message %1").arg(QString(buf.toHex()));
    }

    // Format the output

    QString qdesc = QString(desc).arg((int) channel+(tag==TAG_SPECIAL?0:1));

    if(tag == TAG_PITCH_BEND && buf.length() == 3) { // recombine LSB/MSB for pitch bend
        return qdesc.arg((int) ((uint8_t)buf[1]) | ((uint8_t) buf[2])<<7 );
    }

    int i = 1;
    while(qdesc.contains("%") && i < buf.length()) {
      qdesc = qdesc.arg((int) buf[i++]);
    }
    return qdesc;
}

QString Bridge::applyTimeStamp(QString message)
{
    QTime now = QTime::currentTime();
    float msecs = this->attachTime.msecsTo(now) / 1000.0;
    return QString("+%1 - %2").arg(msecs, 4).arg(message);
}

