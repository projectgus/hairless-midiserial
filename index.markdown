Title: The Hairless MIDI<->Serial Bridge
CSS: markdown.css

<p class="centered"><img src="images/logo.png" alt="Hairless MIDISerial Bridge Logo" /></p>

Hairless MIDI<->Serial Bridge is the easiest way to connect serial devices (like Arduinos) to send and receive MIDI signals. 100% Free Software. For Mac OS X, Windows & Linux.

<p class="centered"><img src="images/diagram.png" alt="Device to Bridge to MIDI Goodness" /></p>

<p class="centered"><a href="#downloads">Download</a> ⊕ <a href="#system_requirements">System Requirements</a> ⊕ <a href="#getting_started">Getting Started</a> ⊕ <a href="#FAQ">FAQ &amp; Troubleshooting</a></p>

<img src="images/windows.png" alt="Screenshot on Windows" />

## Why Use Hairless MIDI<->Serial?

* Cross platform - same program on OS X, Windows, Linux.

* Has no dependencies, can be run out of the box on nearly any computer.

* Improves FTDI latency on Linux & Windows.  This means better MIDI latency when using FTDI-based devices like the Arduino Duemilanove.

* Compatible with the  [Arduino MIDI Library](http://arduino.cc/playground/Main/MIDILibrary) ([see tip](#how_do_i_use_the_arduino_midi_library).)

* Compatible with the Linux program  [ttymidi](http://www.varal.org/ttymidi/), and their Ardumidi library (included in the download.)


## Downloads

* [Hairless MIDI<->Serial Bridge for Mac OS X](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.4-macosx.zip)

* [Hairless MIDI<->Serial Bridge for Windows](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.4-windows.zip)

* [Hairless MIDI<->Serial Bridge for Linux](https://github.com/downloads/projectgus/hairless-midiserial/hairless-midiserial-0.4-linux.tgz)


## System Requirements

* **OS X** 10.4 or later, either PowerPC or Intel based Macs.

* **Linux**, any common x86 or x64 (with 32-bit compatibility libraries) Linux distribution.

    If you're a command line user in Linux, you may want to try the very nice [ttymidi](http://www.varal.org/ttymidi/) command line bridge program instead.

* **Windows** XP or later, 32- or 64-bit.

    NB: To convey Windows MIDI data from one bit of software to another, you'll need a Virtual MIDI passthrough driver. We recommend [loopMIDI](http://www.tobias-erichsen.de/software/loopmidi.html), or there is alternatively the older [MIDI Yoke](http://www.midiox.com/myoke.htm) (MIDI Yoke is not recomended for Vista or newer.)

* **Windows Vista, 7 & 8** special notes:

    To use Hairless MIDI<->Serial Bridge's automatic FTDI latency reduction feature, you may need to "Run As Administrator" when you launch the program.



## Demo

I never got around to making a demo video, but Greg was kind enough to let me link [his Arduino tone synth](http://arduino.cc/forum/index.php?topic=79326.0) video here:

<iframe width="420" height="315" src="http://www.youtube.com/embed/4fUH5rKpH78" frameborder="0">

</iframe>

## History


* Version 0.4 - 7 August 20102

    When using 'Debug MIDI Messages', channel numbers now range 1-16 not 0-15, as per MIDI standard.

    Add two-way (output to serial port) support for SysEx, time & sense messages.

    Added latest & final version of Ardumidi library (Ardumidi is now longer being developed, another
    option is the [Arduino MIDI Library](http://arduino.cc/playground/Main/MIDILibrary).)

    (Linux) fixed a bug where 'ACM' type USB serial devices (like Arduino UNO) weren't recognised.

* Version 0.3 - 8 May 2012

	 Ardumidi updated to work with Arduino 1.0 (thanks to Jorge Falcão.)

    Fixed bug with receiving simultaneous MIDI notes.

    Added support for running MIDI status, as used by Arduino MIDI Library.

    Now supports variable-length SysEx ("System Extension") data messages.

* Version 0.2 - 28 September 2011

    Fixed issue where UI could become overloaded if serial device sent excess data.

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
connect to the same loopMIDI virtual MIDI port.)

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


### How do I use the Arduino MIDI Library?

The [Arduino MIDI Library](http://arduino.cc/playground/Main/MIDILibrary) is a newer
MIDI library than Ardumidi, and seems to be the way of the future.

To use this library with Hairless Bridge, you can mostly follow their
instructions exactly. The only difference is that you need to
initialise the Serial port after the MIDI library, like this:

    void setup() {
      MIDI.begin();
      Serial.begin(115200);
    }


### I can't reprogram my Arduino any more

You need to disable the Serial<->MIDI Bridge before sending other data
(like Arduino programming) to the serial port. You can do this by
unchecking the "Serial<->MIDI Bridge" checkbox in the interface.


### I'm not receiving any data

* Check the serial port settings (like baud rate) are correct (select
  Preferences from the menu.)

* In the serial port settinhgs dialog, try enabling or disabling
  Hardware Flow Control (you may need to restart the bridge after
  changing this setting).

* Try using a Serial Terminal program like minicom or Hyperterminal
  and see if any data is received from there.


### My MIDI program (Ableton, Logic) doesn't show up in the dropdown

##### ... On OS X

You may need to enable the "IAC" virtual MIDI port. This makes a
"channel" that can join the two together:
* Open the OS X built-in "Audio MIDI Setup" application (under Applications->Utilities.)
* Choose "Show MIDI Window" from the Window menu.
* Double-click the "IAC Driver" in the MIDI window.
* Check the "Device is online" box in the dialog that appears.
* Go back to Hairless MIDI<->Serial and there should be a new "IAC Bus" option in the dropdown. The other MIDI program should also see the IAC bus option.

##### ... On Windows

You need to create a loopMIDI virtual port (there's a link under <a
href="#system_requirements">System Requirements</a>) to create a
"channel" that joins the two together.

After intalling loopMIDI, launch it and click the "+" button to create a new virtual MIDI port with the specified name. Both Hairless Bridge and your MIDI program should then connect to this port.

##### ... On Linux

You may need to use the "Passthrough" port in Linux ALSA to join them
together, or use the "Create Virtual MIDI channel" feature if it
exists in your other program.


### What similar software is available?

* [ttymidi](http://www.varal.org/ttymidi/) command-line bridge on Linux.
* [Serial-MIDI Converter](http://spikenzielabs.com/SpikenzieLabs/Serial_MIDI.html) on OS X, Windows, Linux untested. Requires Java.
* [midiserial](http://blipbox.org/blog/projects/midiserial/) another command-line bridge. Only tested on OS X.
* Some microcontrollers, such as [Arduino Uno](http://code.google.com/p/hiduino/) and [Teensy](http://www.pjrc.com/teensy/td_midi.html) are capable of being proper USB MIDI devices, so you don't need a serial bridge.


### Can I use the native MIDI baud rate, 31250bps?

It would be nice to be able to use the hardware MIDI baud rate,
31250bps. This means the logical signals from Hairless Bridge can be
used with real MIDI hardware, with just some electronics to adjust the
serial voltages to become MIDI current loop signals.

Unfortunately, most computer serial ports can't actually talk at
31250bps. For historical & technical reasons they're usually limited
to multiples of 300bps.

However, if you're prepared to hack around a bit, the FTDI USB/Serial
chip (as found on the Arduino Duemilanove) can talk the native MIDI
rate. Have a read through [these Arduino forum
posts](http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1161023163) and
you can see how to hack the driver on OS X or Windows, so that when
you choose a different rate (like 38400bps) it is actually 31250,
behind the scenes.

This is not a supported configuration for Hairless Bridge, but it can
be made to work in some circumstances.


### Where did the name come from?

Do you see any [hairy yaks](http://projects.csail.mit.edu/gsb/old-archive/gsb-archive/gsb2000-02-11.html) around
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

<!-- Piwik -->
<script type="text/javascript">
var pkBaseURL = (("https:" == document.location.protocol) ? "https://stats.projectgus.com/" : "http://stats.projectgus.com/");
document.write(unescape("%3Cscript src='" + pkBaseURL + "piwik.js' type='text/javascript'%3E%3C/script%3E"));
</script>
<script type="text/javascript">
try {
var piwikTracker = Piwik.getTracker(pkBaseURL + "piwik.php", 2);
piwikTracker.trackPageView();
piwikTracker.enableLinkTracking();
} catch( err ) {}
</script>
<noscript><p><img src="http://stats.projectgus.com/piwik.php?idsite=2" style="border:0" alt="" /></p></noscript>
<!-- End Piwik Tracking Code -->
