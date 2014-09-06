/*
 * Linux implementation of fixing latency on a serial port.
 *
 * qextserialport doesn't provide an easy way to get the USB details from the serial port, to find FTDIs.
 *
 * However, Linux has a much better serial flag called ASYNC_LOW_LATENCY that will do the job, anyhow.
 * ... and maybe work on other serial port types!
 */

#include "PortLatency.h"

extern "C" {
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
}

void PortLatency::fixLatency()
{
    if(swapAsync(true)) {
        emit debugMessage(QString("Switched serial port %1 to low-latency mode").arg(portName));
        this->isSwitched = true;
    }
}

void PortLatency::resetLatency()
{
    if(swapAsync(false)) {
        emit debugMessage(QString("Set serial port %1 back to normal mode").arg(portName));
        this->isSwitched = false;
    }
}

bool PortLatency::swapAsync(bool setAsyncMode)
{
    if(isSwitched == setAsyncMode) {
        return true;
    }
    if(!portName.startsWith("/dev/")) {
        portName.prepend("/dev/");
    }
    int fd = open(portName.toAscii().data(), O_NONBLOCK);
    if(fd <= 0) {
        emit errorMessage(QString("Failed to open serial port device %1 to lower latency").arg(portName));
        return false;
    }
    struct serial_struct ser_info;
    int r = ioctl(fd, TIOCGSERIAL, &ser_info);
    if(r) {
        emit errorMessage(QString("Failed to get serial port info for %1, cannot set low latency mode").arg(portName));
        close(fd);
        return false;
    }
    if((ser_info.flags & ASYNC_LOW_LATENCY) == setAsyncMode) {
        emit debugMessage(QString("Serial port %1 already%2 in low latency mode")
                          .arg(portName).arg(setAsyncMode ? "" : " not"));
        close(fd);
        return false;
    }
    if(setAsyncMode) {
        ser_info.flags |= ASYNC_LOW_LATENCY;
    } else {
        ser_info.flags &= ~ASYNC_LOW_LATENCY;
    }
    r = ioctl(fd, TIOCSSERIAL, &ser_info);
    if(r) {
        emit errorMessage(QString("Failed to set low-latency mode on %1").arg(portName));
        close(fd);
        return false;
    }
    close(fd);
    return true;
}
