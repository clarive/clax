#!/bin/sh

if which git && [ -d ".git" ]; then
    VERSION="`date +%Y%m%d`-`git rev-parse --short HEAD`"

    cp clax_version.h.template clax_version.h
    sed -i "s/CLAX_VERSION \".*\"/CLAX_VERSION \"$VERSION\"/" clax_version.h

    echo $VERSION > VERSION
fi

if [ "$OS" = "" ] && [ "$ARCH" = "" ]; then
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
fi

sed "s/CLAX_OS \".*\"/CLAX_OS \"$OS\"/" clax_version.h > clax_version.h.2
mv clax_version.h.2 clax_version.h
sed "s/CLAX_ARCH \".*\"/CLAX_ARCH \"$ARCH\"/" clax_version.h > clax_version.h.2
mv clax_version.h.2 clax_version.h
