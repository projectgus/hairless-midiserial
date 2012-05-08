/*
 * Windows implementation of fixing latency on an FTDI-based serial port, using registry.
 *
 */

#include "PortLatency.h"
#include "qextserialport/qextserialenumerator.h"
extern "C" {
#include <windows.h>
}

// Return the windows registry entry for the FTDI device (open), or 0
HKEY PortLatency::openFTDIDeviceParameters()
{
    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath,
                            0, KEY_READ, &key);
    if(r) {
        emit debugMessage("FTDI drivers don't seem to be installed. Not attempting any FTDI latency tricks.");
        return 0;
    }

    // Look through available FTDI devices
    int index = 0;
    HKEY readOnlyKey = 0, paramKey = 0;
    while(1) {
        if(paramKey) {
          RegCloseKey(paramKey);
          paramKey = 0;
        }
        DWORD subkey_sz = 140;
        wchar_t subkey[subkey_sz];
        r = RegEnumKeyEx(key, index++, subkey, &subkey_sz, 0, NULL, NULL, NULL);        
        if(r) {
            if(r==ERROR_NO_MORE_ITEMS)
                emit debugMessage(QString("Port %1 doesn't seem to be an FTDI device. No latency tricks will be attempted.").arg(portName));
            else
                emit errorMessage(QString("Unexpected error while enumerating FTDI devices."));
            break;
        }
        QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
        r = RegOpenKeyEx(key, paramPath.toStdWString().c_str(), 0, KEY_READ, &readOnlyKey);
        if(r) {
            continue;
        }
        DWORD regPortName_sz = 10;
        wchar_t regPortName[regPortName_sz];
        DWORD type;
        r = RegQueryValueEx(readOnlyKey, L"PortName", NULL, &type, (LPBYTE)&regPortName, &regPortName_sz);
        if(!r && portName == QString::fromWCharArray(regPortName)) {
           r = RegOpenKeyEx(readOnlyKey, NULL, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
           if(r) {
               emit errorMessage("Port is an FTDI device, but we don't have administrator access to change the latency setting.");
           }
        }
        RegCloseKey(readOnlyKey);
        if(paramKey) break;
    }
    RegCloseKey(key);
    return paramKey;
}

/*
 * Read a DWORD value from one registry value under paramKey and copy it to another value under paramKey
 */
bool PortLatency::switchLatency(HKEY paramKey, const wchar_t *readFromKey, const wchar_t *writeToKey, bool checkValue)
{
    DWORD latency, type, lsize = sizeof(DWORD);
    int r = RegQueryValueEx(paramKey, readFromKey, NULL, &type, (LPBYTE)&latency, &lsize);
    if(r) {
        emit errorMessage(QString("Failed to read the %1 value for FTDI device %2").arg(QString::fromWCharArray(readFromKey)).arg(portName));
        return false;
    }
    if(type != REG_DWORD) {
        emit errorMessage(QString("%1 of %2 has unexpected type %3").arg(QString::fromWCharArray(readFromKey)).arg(portName).arg(type));
        return false;
    }
    if(checkValue && latency == 1) {
        emit debugMessage(QString("LatencyTimer for %1 has already been turned down to %2").arg(portName).arg(latency));
        return false;
    }
    r = RegSetValueEx(paramKey, writeToKey, 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
    if(r) {
        emit errorMessage(QString("Failed to write key %1. Missing privileges?").arg(QString::fromWCharArray(writeToKey)));
        return false;
    }
    return true;
}

void PortLatency::fixLatency()
{
    if(isSwitched) return;
    HKEY paramKey = openFTDIDeviceParameters();
    if(!paramKey) return;

    // if we get here, paramKey is the subkey with Device Parameters for our port

    if(switchLatency(paramKey, L"LatencyTimer", L"OldLatencyTimer", true))
    {
        DWORD latency = 1;
        int r = RegSetValueEx(paramKey, L"LatencyTimer", 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
        isSwitched = (r == 0);
    }
    RegCloseKey(paramKey);
}

void PortLatency::resetLatency()
{
    if(!isSwitched) return;
    HKEY paramKey = openFTDIDeviceParameters();
    if(!paramKey) return;

    // if we get here, paramKey is the subkey with Device Parameters for our port

    isSwitched = !switchLatency(paramKey, L"OldLatencyTimer", L"LatencyTimer", false);
    if(!isSwitched)
    {
        int r = RegDeleteValue(paramKey, L"OldLatencyTimer");
        if(r) {
            emit debugMessage("Failed to delete OldLatencyTimer key after switching back. You can probably ignore this message.");
        }
    }
    RegCloseKey(paramKey);
}
