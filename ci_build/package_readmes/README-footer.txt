ARDUINO LIBRARIES
=================

Hairless Midiserial is compatible with the "Arduino MIDI Library",
which is available from http://arduino.cc/playground/Main/MIDILibrary

When using Arduino MIDI Library with Hairless Midiserial (rather than a "real"
MIDI interface), the only difference is you need to initialise the serial port
-after- calling MIDI.begin(), like this:

void setup() {
  MIDI.begin(1);
  Serial.begin(115200);
}

Set the baud rate in Serial.begin() to the same rate being used in Hairless Midiserial.


USING ARDUMIDI
==============

Ardumidi is an older Arduino MIDI library, part of the separate ttymidi
project (http://www.varal.org/ttymidi/). It can also be used with Hairless
MIDI<->Serial Bridge.

The ttymidi authors now recommend using Arduino MIDI Library instead of
Ardumidi. The only notable feature of ttymidi is it supports sending
(non-standard) debugging messages in the serial output, which both Hairless
Midiserial and ttymidi can display.

To install Ardumidi, place the "Ardumidi" directory inside a
"libraries" directory within your Arduino "sketchbook" directory. Then
restart the Arduino IDE. Example sketches can be found in the example/ folder.


LICENSE
=======

The Hairless MIDI<->Serial Bridge is licensed under the GNU Lesser
General Public License 2.1, as described in the About box of the
application and also the included license file.
