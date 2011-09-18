/*
  This file is public domain. Use it as you like.
  
  This is an example program to showcase reading  with 
  Arduino the MIDI output from an ALSA sequencer. This simply
  toggles the Arduino LED whenever a MIDI "note on" or "note off"
  event takes place.
*/

#include <ardumidi.h>

void setup()
{
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
}

int counter = 0;

void loop()
{
	while (midi_message_available() > 0) {

		MidiMessage m = read_midi_message();

		if (m.command == MIDI_NOTE_ON) {
			/* 
			If you want to find out which key was pressed, check
			the value of m.param1
			*/
			digitalWrite(ledPin, HIGH);
	    } 

		else if (m.command == MIDI_NOTE_OFF) {
			digitalWrite(ledPin, LOW);
		}

	}
}
