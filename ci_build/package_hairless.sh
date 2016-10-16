#!/bin/bash
set -xeou pipefail

cd $TRAVIS_BUILD_DIR

case $TARGET in
    linux64)
        BINARY=hairless-midiserial
        ;;
    linux32)
        BINARY=hairless-midiserial
        ;;
    win32)
        BINARY=release/hairless-midiserial.exe
        ;;
    macos)
        BINARY=hairless-midiserial.app
        ;;
    *)
        echo "Target not found: $TARGET"
        exit 1
        ;;
esac

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

