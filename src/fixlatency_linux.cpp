/*
 * Linux implementation of fixing latency on a serial port.
 *
 * qextserialport doesn't provide an easy way to get the USB details from the serial port, to find FTDIs.
 *
 * However, Linux has a much better serial flag called ASYNC_LOW_LATENCY that will do the job, anyhow.
 * ... and maybe work on other serial port types!
 */

#include "fixlatency.h"

extern "C" {
#include <sys/ioctl.h>
#include <linux/serial.h>
#include <linux/ioctl.h>
#include <asm/ioctls.h>
#include <stdio.h>
#include <fcntl.h>
}

bool fixLatency(QString portPhysName)
{
    int fd = open(portPhysName.toAscii().data(), O_NONBLOCK);
    if(fd <= 0)
        return false;
    struct serial_struct ser_info;
    int r = ioctl(fd, TIOCGSERIAL, &ser_info);
    if(!r) {
        ser_info.flags |= ASYNC_LOW_LATENCY;
        r = ioctl(fd, TIOCSSERIAL, &ser_info);
    }
    close(fd);
    return r == 0;
}
