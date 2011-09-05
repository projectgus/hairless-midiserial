/*
 * Windows implementation of fixing latency on an FTDI-based serial port, using registry.
 *
 */

#include "fixlatency.h"
#include "qextserialport/qextserialenumerator.h"
#include <winreg.h>

static bool findUSBIds(QString physName, int *vendorId, int *productId)
{
    QList<QextPortInfo> ports = QextSerialEnumerator::getPorts();
    for(QList<QextPortInfo>::iterator it = ports.begin(); it != ports.end(); it++) {
        if(it->physName == physName) {
            *vendorId = it->vendorID;
            *productId = it->productID;
            return true;
        }
    }
    return false;
}

bool fixLatency(QString portPhysName, QString &errorMessage)
{
    int vid = 0,pid = 0;

    if(!findUSBIds(portPhysName, &vid, &pid) || vid == 0 || pid == 0) {
        errorMessage.sprintf("Not a USB device");
        return false; // Not USB, so definitely not FTDI
    }

    HKEY key;
    const wchar_t *keypath = L"SYSTEM\\CurrentControlSet\\Enum\\FTDIBUS";
    long r = RegOpenKeyEx(HKEY_LOCAL_MACHINE, keypath,
                            0, KEY_READ, &key);
    if(r) {
        errorMessage.sprintf("Failed to open parent FTDIBUS key. No FTDI driver?");
        return false;
    }

    // subkeys of FTDIBUS look like VID_0403+PID_6001+A6006eL7A
    QString prefix = QString("VID_%1+PID_%2").arg(vid,4,16,QChar('0')).arg(pid,4,16,QChar('0'));

    // Look through available subkeys
    int index = 0;
    const int SUBKEY_SZ = 140;
    wchar_t subkey[SUBKEY_SZ];
    while(r == 0) {
        DWORD keysz = SUBKEY_SZ;
        r = RegEnumKeyEx(key, index++, subkey, &keysz, 0, NULL, NULL, NULL);
        if (r == 0 && QString::fromWCharArray(subkey).startsWith(prefix))
            break;
    }

    if(r == ERROR_NO_MORE_ITEMS) {
        errorMessage.sprintf("Not found, prefix %s tried %d alternatives", prefix.toAscii().data(), index);
        RegCloseKey(key); // Not found
        return false;
    }
    else if(r) {
        errorMessage.sprintf("Failed to enumerate FTDIBUS keys");
        RegCloseKey(key);
        return false;
    }

    HKEY paramKey = 0;
    QString paramPath = QString("%1\\0000\\Device Parameters").arg(QString::fromWCharArray(subkey));
    if(paramPath.length() >= SUBKEY_SZ) {
        errorMessage.sprintf("Internal error: Subkey path %s is too long for buffer", paramPath.toAscii().data());
        RegCloseKey(key);
        return false;
    }
    paramPath.toWCharArray(subkey);
    r = RegOpenKeyEx(key, subkey, 0, KEY_READ|KEY_SET_VALUE, &paramKey);
    if(r) {
        errorMessage.sprintf("Failed to open subkey %s (error %lx)", QString::fromWCharArray(subkey).toAscii().data(), r);
        RegCloseKey(key);
        return false;
    }

    DWORD latency, type, lsize = sizeof(DWORD);
    r = RegQueryValueEx(paramKey, L"LatencyTimer", NULL, &type, (LPBYTE)&latency, &lsize);
    if(r) {
        errorMessage.sprintf("Failed to open LatencyTimer value %s", paramPath.toAscii().data());
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
    RegCloseKey(key);
    return r == 0;
}
