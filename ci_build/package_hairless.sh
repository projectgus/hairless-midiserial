#!/bin/bash
set -xeou pipefail

cd $TRAVIS_BUILD_DIR

case $TARGET in
    linux64)
        BINARY=hairless-midiserial
        APPIMAGE=hairless-midiserial-x86_64.AppImage
        ;;
    linux32)
        BINARY=hairless-midiserial
        APPIMAGE=hairless-midiserial-i386.AppImage
        ;;
    win32)
        BINARY=release/hairless-midiserial.exe
        APPIMAGE=
        ;;
    macos)
        BINARY=hairless-midiserial.app
        APPIMAGE=
        ;;
    *)
        echo "Target not found: $TARGET"
        exit 1
        ;;
esac

if [ -n "$APPIMAGE" ]; then
    # Build an AppImage!

    # This is necessary because the "static linked" hairless binary currently
    # still links against a bunch of X11 related shared libraries, can't find
    # a way to build Qt with these as static libraries.

    if [ ! -f "$LINUXDEPLOY_DIR/$LINUXDEPLOY_BIN" ]; then
        mkdir -p $LINUXDEPLOY_DIR
        cd $LINUXDEPLOY_DIR
		wget "$LINUXDEPLOY_URL"
		chmod +x "$LINUXDEPLOY_BIN"
        cd -
	fi

    # TODO: do we need qt plugin?
    # TODO: copyright file(s)

    mkdir AppDir
    # linuxdeploy will assemble the parts we provide into the AppDir structure and then build the AppImage

    "$LINUXDEPLOY_DIR/$LINUXDEPLOY_BIN" \
                                        -e $BINARY \
                                        -d ci_build/hairless-midiserial.desktop \
                                        -i resources/images/hairless-midiserial-icon.png \
                                        --appdir AppDir \
                                        --output appimage
    BINARY="*.AppImage"
fi

VERSION=$(git describe --tags --always || get rev-parse HEAD)

PACKAGE_DIR="hairless-midiserial-${TARGET}-${VERSION}"

mkdir $PACKAGE_DIR

cp -r $BINARY $PACKAGE_DIR

# assemble a README for this platform, with the version number set
sed s/@VERSION@/`git describe --tags`/ \
    ci_build/package_readmes/README-header.txt \
    ci_build/package_readmes/README-${TARGET}.txt \
    ci_build/package_readmes/README-footer.txt \
    > $PACKAGE_DIR/README.txt

cp LICENSE $PACKAGE_DIR

if [ "$TARGET" == "win32" ] || [ "$TARGET" == "macos" ]; then
    zip -r "${PACKAGE_DIR}.zip" "${PACKAGE_DIR}"
else
    tar -zcvf "${PACKAGE_DIR}.tgz" "${PACKAGE_DIR}"
fi

