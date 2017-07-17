#!/bin/bash

if [ -f OS ]; then
    OS=$(tr -d "\012" < OS)
fi

if [ -f ARCH ]; then
    ARCH=$(tr -d "\012" < ARCH)
fi

. util/detect-os.sh

VERSION=$(tr -d "\012" < VERSION)

DIST="clax_${VERSION}_${OS}-${ARCH}";

if [ "$WINDOWS" = "1" ]; then
    ARCHIVE="$DIST.zip"
    rm -rf "${DIST:?}/"
    mkdir "$DIST"

    cp clax.exe "$DIST/"
    cp VERSION "$DIST/"
    cp contrib/wininetd/* "$DIST/"
    cp clax.ini.win.example "$DIST/clax.ini"

    zip -r "$ARCHIVE" "$DIST/" || exit 255
else
    TAR="$DIST.tar"
    ARCHIVE="$TAR.gz"
    rm -rf "${DIST:?}/"
    mkdir "$DIST"

    cp clax "$DIST/"
    cp VERSION "$DIST/"
    cp clax.ini.unix.example "$DIST/clax.ini"

    tar cf "$TAR" "$DIST/" || exit 255
    gzip -f "$TAR" || exit 255
fi

echo
echo "DONE"
echo
echo "$ARCHIVE"
