/*
    This file is part of ttymidi.

    ttymidi is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ttymidi is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ttymidi.  If not, see <http://www.gnu.org/licenses/>.
*/
// file version 0.60

#include "ardumidi.h"
#include "HardwareSerial.h"

void midi_note_off(byte channel, byte key, byte velocity)
{
	midi_command(0x80, channel, key, velocity);
}

void midi_note_on(byte channel, byte key, byte velocity)
{
	midi_command(0x90, channel, key, velocity);
}

void midi_key_pressure(byte channel, byte key, byte value)
{
	midi_command(0xA0, channel, key, value);
}

void midi_controller_change(byte channel, byte control, byte value)
{
	midi_command(0xB0, channel, control, value);
}

void midi_program_change(byte channel, byte program)
{
	midi_command_short(0xC0, channel, program);
}

void midi_channel_pressure(byte channel, byte value)
{
	midi_command_short(0xD0, channel, value);
}

void midi_pitch_bend(byte channel, int value)
{
	midi_command(0xE0, channel, value & 0x7F, value >> 7);
}

void midi_command(byte command, byte channel, byte param1, byte param2)
{
	Serial.print(command | (channel & 0x0F), BYTE);
	Serial.print(param1 & 0x7F, BYTE);
	Serial.print(param2 & 0x7F, BYTE);
}

void midi_command_short(byte command, byte channel, byte param1)
{
	Serial.print(command | (channel & 0x0F), BYTE);
	Serial.print(param1 & 0x7F, BYTE);
}

void midi_print(char* msg, int len)
{
	Serial.print(0xFF, BYTE);
	Serial.print(0x00, BYTE);
	Serial.print(0x00, BYTE);
	Serial.print(len , BYTE);
	Serial.print(msg);
}

void midi_comment(char* msg)
{
	int   len = 0;
	char* ptr = msg;
	while (*ptr++) len++;
	midi_print(msg, len);
}

int midi_message_available() {
	/* 
	   This bit will check that next bytes to be read would actually
	   have the midi status bit. If not it will remove uncorrect bytes
	   from internal buffer 
	   */
	while ((Serial.available() > 0) && ((Serial.peek() & B10000000) != 0x80)) {
		Serial.read();
	}

	/* Well we don't exactly know how many commands there might be in the Serial buffer
           so we'll just guess it according the type of message that happens to be waiting
           in the buffer. At least we get first one right! */
	byte command = Serial.peek() & 11110000;
	if (command != MIDI_PROGRAM_CHANGE && command != MIDI_CHANNEL_PRESSURE) {
		return (Serial.available()/2);
	}
	return (Serial.available()/3);
}

MidiMessage read_midi_message() {
	MidiMessage message;
	byte midi_status = Serial.read();
	message.command  = (midi_status & B11110000);
	message.channel  = (midi_status & B00001111);
	message.param1   = Serial.read();
	if (message.command != MIDI_PROGRAM_CHANGE && message.command != MIDI_CHANNEL_PRESSURE) {
		message.param2   = Serial.read();
	}
	return message;
}

int get_pitch_bend(MidiMessage m) {
	return (m.param1 & 0x7F) + ((m.param2 & 0x7F) << 7);
}

