#!/bin/bash

if [ "$WINDOWS" = "1" ]; then
    OS=windows
    ARCH=x86_64
elif uname -a | grep 'OS/390'; then
    OS=zos
    ARCH=`uname -m`
else
    OS=linux
    ARCH=`uname -m`
fi

VERSION=`cat VERSION | tr -d "\012"`

DIST="clax_${VERSION}_${OS}-${ARCH}";

if [ "$WINDOWS" = "1" ]; then
    ARCHIVE="$DIST.zip"
    rm -rf $DIST/
    mkdir $DIST

    cp clax.exe $DIST/
    cp contrib/wininetd/* $DIST/
    cp clax.ini.win.example $DIST/clax.ini

    zip -r $ARCHIVE $DIST/
else
    ARCHIVE="$DIST.tar.gz"
    rm -rf $DIST/
    mkdir $DIST

    cp clax $DIST/
    cp clax.ini.unix.example $DIST/clax.ini

    tar czf $ARCHIVE $DIST/
fi

echo
echo "DONE"
echo
echo $ARCHIVE
