Hairless MIDI<->Serial Bridge is the easiest way to connect serial devices (like Arduinos) to send and receive MIDI signals.

The project home page is https://projectgus.github.io/hairless-midiserial/

Please see that page for information on running and using Hairless Midiserial, and downloadable compiled versions for Windows, OS X and Linux.

# Alternatives

For a tool which runs on the command line, I suggest checking out
ttymidi - http://www.varal.org/ttymidi/ - a different project that
creates a Serial/MIDI bridge on the command line. Hairless Midiserial is compatible with ttymidi.

# Building Hairless Midiserial from source

[Prebuild versions are available for download](https://projectgus.github.io/hairless-midiserial/#downloads), but it's also possible to build Hairless Midiserial from source if you have [Qt](https://www.qt.io/) installed:

* Hairless uses git submodules for library dependencies, so you should use `git clone --recursive URL` when cloning from Github. Alternatively, you can run `git submodule update --init` in order to fetch the submodules to an already-cloned directory.

* Hairless Midiserial uses Qt 5. The Travis Continuous Integration system builds with a particular version of Qt, check the .travis.yml file for the current exact Qt version used for Travis builds.

* The graphical IDE "Qt Creator" or the program "qmake" can be used to compile the project hairless-midiserial.pro.

# Libraries

* [qextserialport](https://github.com/qextserialport/qextserialport/) is hosted on Github and is linked into the source tree as a git submodule. (Note: this library is no longer maintained and [Qt now supplies a serial port class](http://doc.qt.io/qt-5/qtserialport-index.html).)

* [The RtMidi library](https://github.com/thestk/rtmidi) is hosted on Github and is linked into the source tree as a git submodule.

Both libraries are small so they compiled as source files directly into Hairless Midiserial, not linked as libraries.

# Release builds & Travis

The official release builds are static linked. They have been built against Qt versions that were compiled from source, configured with `-static` for static linking.

The [Travis CI system](https://travis-ci.org/projectgus/hairless-midiserial/) builds a static linked Qt (with unneccesary modules and extras removed where possible) for Windows, Linux (32-bit and 64-bit) and macOS, and then uses this Qt to build release-ready versions of Hairless Midiserial. See `.travis.yml` and the `ci_build/build_qt_static.sh` script for all details.

(Many thanks to the [Travis CI folks](https://travis-ci.com) who provide this service to open source projects for free.)
