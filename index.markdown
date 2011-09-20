---
title: The Hairless MIDI<->Serial Bridge
---
<link href="markdown.css" rel="stylesheet"></link>

<p class="centered"><img src="images/logo.png" alt="Hairless MIDISerial Bridge Logo" /></p>

Hairless MIDI<->Serial Bridge is a free software tool that makes it easy to connect serial devices (like Arduinos) to send and receive MIDI signals. For Mac OS X, Windows & Linux.

<p class="centered"><img src="images/diagram.png" alt="Device to Bridge to MIDI Goodness" /></p>

<p class="centered"><a href="#system_requirements">System Requirements</a>  <a href="#downloads">Download</a>  <a href="#getting_started">Getting Started</a>  <a href="#FAQ">FAQ</a></p>

<img src="images/windows.png" alt="Screenshot on Windows" />

## Why Use Hairless MIDI<->Serial?

* Cross platform - same program on OS X, Windows, Linux.

* Has no dependencies, can be run out of the box on nearly any computer.

* Improves FTDI latency on Linux & Windows.  This means better MIDI latency when using FTDI-based devices like the Arduino Duemilanove.

* Fully compatible with the Linux program  [ttymidi](http://www.varal.org/ttymidi/), and their Ardumidi library (included in the download.)

## System Requirements

* **OS X** 10.4 or later, either PowerPC or Intel based Macs.

* **Linux**, any common x86 or x64 (with 32-bit compatibility libraries) Linux distribution.

    * If you're a command line user in Linux, you may want to try the very nice [ttymidi](http://www.varal.org/ttymidi/) command line bridge program instead.

* **Windows** XP or later, 32- or 64-bit. 

    NB: To convey Windows MIDI data from one bit of software to another, you'll need a Virtual MIDI passthrough driver. We recommend [MIDI Yoke](http://www.midiox.com/myoke.htm).

* **Windows Vista** or **Windows 7** special notes:

    To use Hairless MIDI<->Serial Bridge's automatic FTDI latency reduction feature, you may need to "Run As Administrator" when you launch the program.

    For help installing MIDI Yoke on Vista or 7, see [here](http://help.touch-able.com/kb/installation/midi-yoke-installation-on-windows-vista-or-windows-7) and [here](http://www.midiox.com/cgi/yabb/YaBB.pl?board=myokent;action=display;num=1299166667).


## Downloads

* [Hairless MIDI<->Serial Bridge for Mac OS X](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.1-macosx.zip)

* [Hairless MIDI<->Serial Bridge for Windows](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.1-windows.zip)

* [Hairless MIDI<->Serial Bridge for Linux](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.1-linux.tgz)


## History

* Version 0.1 - Initial release, 20 September 2011.


## Source Code

[Source is all available on github](https://github.com/projectgus/hairless-midiserial).


## Getting Started

1. Launch "Hairless MIDI<->Serial Bridge" by double-clicking the application.

    <img src="images/osx1.png" alt="App icon in OS X" />

2. Plug in your serial MIDI device, if it's not already.

3. Select Preferences from the menu and check that the serial settings
look correct (defaults are 115200bps, no flow control.)

4. Choose your serial port from the "Serial port" box.

    <img src="images/osx2.png" alt="Choosing serial port in OS X" />

6. Launch the music app that will be sending and/or receiving MIDI data.

7. Set your music app to send or receive MIDI data, if it's not enabled already.

8. Select your app's MIDI port from the MIDI dropdown (on Windows, set both sides to
connect to the same MIDI Yoke virtual MIDI port.)

    <img src="images/osx3.png" alt="Up and running in OS X" />

9. Go crazy! You should see lights flashing in the UI when MIDI and/or
serial messages come through.

    You can enable "Debug MIDI messages" if you want to see all of the
    MIDI messages as they come through the bridge.

10. To disable the bridge, uncheck the "Serial<->MIDI Bridge" checkbox
at the top of the window.


## FAQ

### I think I found a bug

Please report it via the
[Issues](https://github.com/projectgus/hairless-midiserial/issues)
page on github.

If you're a programmer and you're feeling awesome, I'd love it if you
could submit a github pull request (or a patch) along with the bug
report. :)


### I can't reprogram my Arduino any more

You need to disable the Serial<->MIDI Bridge before sending other data
(like Arduino programming) to the serial port. You can do this by
unchecking the "Serial<->MIDI Bridge" checkbox in the interface.


### I'm not receiving any data

* Check the serial port settings (like baud rate) are correct (select
  Preferences from the menu.) Try disabling Flow Control if it was
  enabled.

* Try using a Serial Terminal program like minicom or Hyperterminal
  and see if any data is received from there.


### What similar software is available?

* [ttymidi](http://www.varal.org/ttymidi/) command-line bridge on Linux.
* [Serial-MIDI Converter](http://spikenzielabs.com/SpikenzieLabs/Serial_MIDI.html) on OS X, Windows, Linux untested. Requires Java.
* [midiserial](http://blipbox.org/blog/projects/midiserial/) another command-line bridge. Only tested on OS X.
* Some microcontrollers, such as [Arduino Uno](http://code.google.com/p/hiduino/) and [Teensy](http://www.pjrc.com/teensy/td_midi.html) are capable of being proper USB MIDI devices, so you don't need a serial bridge.

### Where did the name come from?

Do you see any [hairy
yaks](http://projects.csail.mit.edu/gsb/old-archive/gsb-archive/gsb2000-02-11.html) around
here? No? Hairless it is. :)

That is also where the poor excuse for an icon (supposed to look like clippers) came from. :)


### Why was this initially created?

For a workshop, "[Drop-Dead Simple MIDI Controllers with
Arduino](http://electrofringe.net/2011/program2011/workshop2011/drop-dead-simple-midi-controllers-with-arduino/)",
as part of the Electrofringe 2011 festival in Newcastle, Australia.

While preparing I couldn't find any MIDI<->Serial bridges that were
fully cross-platform, low latency and didn't have setup
dependencies. So I decided to shave a yak and make one. :)


## Libraries

Big thanks to the authors of the Free Software libraries that Hairless
MIDI<->Serial Bridge builds on. Those are [Qt](qt.nokia.com),
[qextserialport](http://code.google.com/p/qextserialport/),
[RtMidi](http://www.music.mcgill.ca/~gary/rtmidi/) and ArduMIDI, from
the [ttymidi](http://www.varal.org/ttymidi/) project.

More details can be found in the About Box of Hairless MIDI<->Serial Bridge.

## Licensing

Hairless MIDI<->Serial Bridge is released under the GNU Lesser General
Public License 2.1, as described in the About Box for compiled
versions and the
[LICENSE](https://github.com/projectgus/hairless-midiserial/blob/master/LICENSE)
file for source versions.

Incorporated libraries are sometimes relicensed with additional terms,
also as described in these places.


This page and the Hairless MIDI<->Serial Bridge are Copyright (c) 2011 Angus Gratton.