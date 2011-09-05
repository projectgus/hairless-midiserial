/*
 * Windows implementation of fixing latency on an FTDI-based serial port, using registry.
 *
 */

#include "fixlatency.h"
#include "qextserialport/qextserialenumerator.h"
#include <winreg.h>

// Return the windows registry entry for the FTDI device (open), or 0
HKEY openFTDIDeviceParameters(QString portName, QString &errorMessage)
{
    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath,
                            0, KEY_READ, &key);
    if(r) {
        errorMessage.sprintf("Failed to open parent FTDIBUS key. No FTDI driver installed?");
        return 0;
    }

    // Look through available FTDI devices
    int index = 0;
    HKEY paramKey = 0;
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
                errorMessage.append("FTDI device not found");
            else
                errorMessage.sprintf("Failed to enumerate FTDIBUS keys");
            break;
        }
        QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
        errorMessage.append(paramPath).append("  ");
        r = RegOpenKeyEx(key, paramPath.toStdWString().c_str(), 0, KEY_READ|KEY_SET_VALUE, &paramKey);
        if(r) {
            continue;
        }
        DWORD regPortName_sz = 10;
        wchar_t regPortName[regPortName_sz];
        memset(regPortName, 0, sizeof(regPortName));
        DWORD type;
        errorMessage.append(" about to load PortName ");
        r = RegQueryValueEx(paramKey, L"PortName", NULL, &type, (LPBYTE)&regPortName, &regPortName_sz);
        if(!r && portName == QString::fromWCharArray(regPortName)) {
            break;
        }
    }
    RegCloseKey(key);
    return paramKey;
}

bool fixLatency(QString portName, QString &errorMessage)
{
    HKEY paramKey = openFTDIDeviceParameters(portName, errorMessage);

    if(!paramKey)
        return false;

    // if we get here, paramKey is the subkey with Device Parameters for our port

    DWORD latency, type, lsize = sizeof(DWORD);
    int r = RegQueryValueEx(paramKey, L"LatencyTimer", NULL, &type, (LPBYTE)&latency, &lsize);
    if(r) {
        errorMessage.sprintf("Failed to read LatencyTimer value");
    } else if(type != REG_DWORD) {
        errorMessage.sprintf("Got unexpected type for LatencyTimer");
    } else if(latency > 1) {
        r = RegSetValueEx(paramKey, L"OldLatencyTimer", 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
        if(r) {
            errorMessage.sprintf("Failed to save old latency value to set back later");
        } else {
            latency = 1;
            RegSetValueEx(paramKey, L"LatencyTimer", 0, REG_DWORD, (LPBYTE)&latency, sizeof(latency));
        }
    }

    RegCloseKey(paramKey);
    return r == 0;
}
