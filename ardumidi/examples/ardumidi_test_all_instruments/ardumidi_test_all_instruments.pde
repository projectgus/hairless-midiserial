/*
  This file is public domain. Use it as you like.
*/

#include <ardumidi.h>

int ledPin = 13;
int note_on = 0;
int voice = 0;

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

void loop()
{
  
  if (!note_on)
  {
	// play a Cminor chord on channel 0, at maximum volume (127)
    midi_note_on(0, MIDI_C, 127);
    midi_note_on(0, MIDI_E + MIDI_FLAT + MIDI_OCTAVE, 127);
    midi_note_on(0, MIDI_G + MIDI_OCTAVE, 127);

	// The MIDI_* macros were created to make your life easier, but you
	// don't have to use them. You may enter numbers instead if you know
	// what you're doing.

    note_on = 1;
    digitalWrite(ledPin, HIGH);
  }

  else
  {
	// stop the Cminor chord
    midi_note_off(0, MIDI_C, 127);
    midi_note_off(0, MIDI_E + MIDI_FLAT + MIDI_OCTAVE, 127);
    midi_note_off(0, MIDI_G + MIDI_OCTAVE, 127);

    // go to next instrument
    voice++;
    if (voice > 255) voice = 0;
    midi_program_change(0, voice);

    note_on = 0;
    digitalWrite(ledPin, LOW);
  }
  
  delay(500);
}

// Available commands:
//
// Start/stop playing a certain note:
//   midi_note_on(byte channel, byte key, byte velocity);
//   midi_note_off(byte channel, byte key, byte velocity);
//
// Change pressure of specific keys:
//   midi_key_pressure(byte channel, byte key, byte value);
//
// Change controller value (used for knobs, etc):
//   midi_controller_change(byte channel, byte controller, byte value);
//
// Change "program" (change the instrument):
//   midi_program_change(byte channel, byte program);
//
// Change key pressure of entire channels:
//   midi_channel_pressure(byte channel, byte value);
//
// Change pitch-bend wheel:
//   midi_pitch_bend(byte channel, int value);
//
// Send a comment:
//   midi_comment(char* str);
//
// Send a series of bytes (to be interpreted by another program):
//   midi_printbytes(char* bytes, int len);
//
// Parameters:
//   channel             an integer from 0 to 15
//   pitch-bend value    an integer from 0 to 16383
//   all other values    an integer from 0 to 127
//

