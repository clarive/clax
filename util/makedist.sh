#!/bin/bash

BRANCH=`git rev-parse --abbrev-ref HEAD`
SHA=`git rev-parse --short HEAD`

DIST="clax-$BRANCH-$SHA";

if [ "$WINDOWS" = "1" ]; then
    DIST="$DIST-win"
    ARCHIVE="$DIST.zip"
    rm -rf $DIST/
    mkdir $DIST

    cp clax.exe $DIST/
    cp contrib/wininetd/* $DIST/
    cp clax.ini.win.example $DIST/clax.ini

    zip -r $ARCHIVE $DIST/
else
    DIST="$DIST-linux"
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
