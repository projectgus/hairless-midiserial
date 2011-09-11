/*
 * Arduino devices' FTDI chips have a LatencyTimer setting of 2ms by default on OS X
 *
 * So there is not much point doing anything at all here.
 *
 * It would be possible to edit the plist file to reduce latency on other devices,
 * but this is a fairly convoluted process (need to scan file ,elevate privileges -
 * including prompt, then write changes) but it seems like overkill when common devices
 * already work.
 */
#include "PortLatency.h"

void PortLatency::fixLatency()
{
}
void PortLatency::resetLatency()
{
}
