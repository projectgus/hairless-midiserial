#ifndef FIXLATENCY_H
#define FIXLATENCY_H

#include <QObject>
#include <QString>
#ifdef Q_OS_WIN32
extern "C" {
#include <windows.h>
}
#endif

/*
 * A simple class to manage latency reduction of FTDI-based serial ports. Implementation varies from
 * OS to OS. Implementation has to determine if port referred to by "portName" actually is
 * an FTDI device or not.
 *
 * Emits debug or status messages to indicate its current status
 *
 */
class PortLatency : public QObject {
    Q_OBJECT
public:
    PortLatency(QString portName);

    void fixLatency();
    void resetLatency();

signals:
    void debugMessage(QString message);
    void errorMessage(QString message);
private:
    QString portName;
    bool isSwitched;

#ifdef Q_OS_WIN32
    bool switchLatency(HKEY paramKey, const wchar_t *readFromKey, const wchar_t *writeToKey, bool checkValue);
    HKEY openFTDIDeviceParameters();
#endif
#ifdef Q_OS_LINUX
    bool swapAsync(bool setAsyncMode);
#endif
};

#endif // FIXLATENCY_H
