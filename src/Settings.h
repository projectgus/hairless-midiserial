#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include "qextserialport.h"

// Note that main.cpp sets QCoreApplication::applicationName and such, allowing us to use a
// plain QSettings() constructor

class Settings {
public:
    static QString getLastSerialPort() { return QSettings().value("lastSerialPort", "").toString(); }
    static void setLastSerialPort(QString port) { QSettings().setValue("lastSerialPort", port); }

    static QString getLastMidiIn() { return QSettings().value("lastMidiIn", "").toString(); }
    static void setLastMidiIn(QString port) { QSettings().setValue("lastMidiIn", port); }

    static QString getLastMidiOut() { return QSettings().value("lastMidiOut", "").toString(); }
    static void setLastMidiOut(QString port) { QSettings().setValue("lastMidiOut", port); }

    static int getScrollbackSize() { return QSettings().value("scrollbackSize", 75).toInt(); }
    static void setScrollbackSize(int newSize) { QSettings().setValue("scrollbackSize", newSize); }

    static bool getDebug() { return QSettings().value("debug", false).toBool(); }
    static void setDebug(bool debug) { QSettings().setValue("debug", debug); }

    static PortSettings getPortSettings() {
        PortSettings result;
        QSettings settings;
        result.BaudRate = (BaudRateType) settings.value("baudRate", (int)BAUD115200).toInt();
        result.DataBits = (DataBitsType) settings.value("dataBits", (int)DATA_8).toInt();
        result.FlowControl = (FlowType)  settings.value("flowControl", (int)FLOW_OFF).toInt();
        result.Parity = (ParityType) settings.value("parity", (int)PAR_NONE).toInt();
        result.StopBits = (StopBitsType) settings.value("stopBits", (int)STOP_1).toInt();
        result.Timeout_Millisec = 1; // not used when qextserialport is event-driven, anyhow
        return result;
    }

    static void setPortSettings(PortSettings newSettings) {
        QSettings settings;
        settings.setValue("baudRate", (int)newSettings.BaudRate);
        settings.setValue("dataBits", (int)newSettings.DataBits);
        settings.setValue("flowControl", (int)newSettings.FlowControl);
        settings.setValue("parity", (int)newSettings.Parity);
        settings.setValue("stopBits", (int)newSettings.StopBits);
    }

};


#endif // SETTINGS_H
