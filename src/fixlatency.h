#ifndef FIXLATENCY_H
#define FIXLATENCY_H

#include <QObject>
#include <QString>

/*
 * A simple method to reduce latency on FTDI-based serial ports. Implementation varies from
 * OS to OS. Implementation has to determine if port referred to by "physName" actually is
 * an FTDI device or not.
 *
 * Returns true on success, false if something went wrong.
 *
 */
bool fixLatency(QString portPhysName, QString &errorMessage);

#endif // FIXLATENCY_H
