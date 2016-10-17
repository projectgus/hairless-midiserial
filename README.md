Hairless MIDI<->Serial Bridge is the easiest way to connect serial devices (like Arduinos) to send and receive MIDI signals.

The project home page is http://projectgus.github.com/hairless-midiserial/

Please see that page for information on running and using Hairless Midiserial, and downloadable compiled versions for Windows, OS X and Linux.

# Building Hairless Bridge from source

(Note again, prebuilt versions *are available for download* at the above URL.)

Hairless uses git submodules for library dependencies, so you should use "git clone --recursive URL" when cloning from github. Alternatively, you can run `git submodule update --init` in order to fetch the submodules to an already-cloned directory.

Hairless Midiserial Bridge release 0.4 was built with Qt 4.7.3. It's also been built and run under Qt 4.7.4 & 4.8.6. Newer Qt version 5.0 will probably require code changes in order to compile and/or run.

The Qt package should contain all dependencies, the graphical IDE "Qt Creator" or the program "qmake" can be used to compile the project hairless-midiserial.pro.

On Windows I recommend building with the [MingGW compiler](http://www.mingw.org/), Visual Studio has not been tested. Neither the MinGW site nor Qt's new owners Digia still distribute older MinGW builds, and MinGW 4.7 is too new for precompiled Qt 4.7.x, so it can be a bit hard to find a prebuilt combination that work. Recently I downloaded mingw-static-4.4.5-all.7z from [this Google Code project](https://code.google.com/p/qp-gcc/downloads/list), and can confirm that works.

(For the Windows release builds I actually [cross-build under Linux using wine, as described here](http://projectgus.com/2011/09/developing-qt-apps-for-windows-using-linux-wine/).)

# Libraries

* [qextserialport](https://code.google.com/p/qextserialport/) is hosted on github and is linked into the source tree as a git submodule.

* [The RtMidi library](https://github.com/thestk/rtmidi) is hosted on github and is linked into the source tree as a git submodule.

Both libraries are small so they compiled as source files directly into Hairless Bridge, not linked as libraries.

# Release builds

The official releases are static linked, so they've actually been built against Qt versions that were compiled from source, configured with "-static" for static linking.
