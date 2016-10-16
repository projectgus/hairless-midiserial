#!/bin/bash
set -xeou pipefail
#
# Shell script to build Qt static, runs as part
# of the Travis continuous integration build.
#
# Variables
# - TRAVIS_BUILD_DIR
# - QTDIR - install pathfor Qt
# - QTURL - URL to Qt .tar.xz file
# - QTPKG - Name of directory unpacked by .tar.xz
# - TARGET - linux64, win32

cd ${TRAVIS_BUILD_DIR}
wget -q "${QTURL}"
tar -Jxf ${QTPKG}.tar.xz

REPOS_SKIP="qt3d qtactiveqt qtandroidextras qtcanvas3d qtcharts qtconnectivity qtconnectivity qtdatavis3d qtdeclarative qtdoc qtgamepad qtgraphicaleffects qtimageformats qtlocation qtlottie qtmacextras qtmultimedia qtnetworkauth qtpurchasing qtquick3d qtquickcontrols qtquickcontrols2 qtquicktimeline qtremoteobjects qtscript qtscxml qtsensors qtserialbus qtspeech qtsvg qttools qttranslations qtvirtualkeyboard qtwayland qtwebchannel qtwebengine qtwebglplugin qtwebsockets qtwebview qtwinextras qtx11extras qtxmlpatterns"

FEATURES_DISABLE="animation bearermanagement big_codecs codecs cssparser cupsjobwidget directfb dnslookup dom dtls ftp gestures graphicseffect graphicsview http im itemmodeltester linux-netlink localserver movie netlistmgr networkdiskcache networkproxy ocsp socks5 sql-db2 sql-ibase sql-mysql sql-oci sql-odbc sql-psql sql-sqlite sql-sqlite2 sql-tds sqlmodel sortfilterproxymodel sspi textmarkdownreader textmarkdownwriter textodfwriter timezone translation treewidget tuiotouch udpsocket undocommand undogroup undostack undoview vnc"

QTCOMMONOPTS="-silent -release -optimize-size -nomake examples -nomake tests  -opensource -static -no-icu -no-opengl -no-ssl -no-libudev -qt-libpng -no-libjpeg -qt-zlib -qt-pcre -qt-freetype -qt-harfbuzz  -confirm-license -prefix ${QTDIR}"

for REPO in $REPOS_SKIP; do
    QTCOMMONOPTS+=" -skip $REPO"
done

for FEATURE in $FEATURES_DISABLE; do
    QTCOMMONOPTS+=" -no-feature-$FEATURE"
done

case $TARGET in
	linux64)
		QTOPTS="-system-xcb -no-feature-pdf -no-feature-printer"
		;;
    linux32)
        QTOPTS="-system-xcb -no-feature-pdf -no-feature-printer -sysroot / -xplatform linux-g++-32"
        ;;
	win32)
		QTOPTS="-static-runtime -no-feature-pdf -no-feature-printer -xplatform win32-g++ -device-option CROSS_COMPILE=i686-w64-mingw32-"
		;;
    macos)
        # macos includes PDF & Printer features to avoid errors compiling QMacPrintEngine
        QTOPTS=""
        ;;
    *)
        echo "Target not found: $TARGET"
        exit 1
        ;;
esac

echo "Applying Qt patches..."

cd ${TRAVIS_BUILD_DIR}/${QTPKG}

PATCHDIRS="${TRAVIS_BUILD_DIR}/ci_build/qt_patches/${TARGET}"
for PATCHDIR in $PATCHDIRS; do
    if [ -d ${PATCHDIR} ]; then
        for PATCH in ${PATCHDIR}/*.patch; do
            echo "Applying ${PATCH}..."
            patch -p1 < ${PATCH}
        done
    fi
done

echo "Configure arguments ${QTCOMMONOPTS} ${QTOPTS}"

mkdir ${TRAVIS_BUILD_DIR}/qt_build
cd ${TRAVIS_BUILD_DIR}/qt_build

# tail config.log on failure, as most of the details are there
${TRAVIS_BUILD_DIR}/${QTPKG}/configure ${QTCOMMONOPTS} ${QTOPTS} || (tail -n 100 config.log && exit 1)

make -j2 install
