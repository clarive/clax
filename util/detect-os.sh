#!/bin/sh

if [ -z "$OS" ]; then
    if [ "$WINDOWS" = "1" ]; then
        export OS=windows
    elif uname -a | grep 'OS/390'; then
        export OS=zos
    else
        export OS=`uname -s | tr '[:upper:]' '[:lower:]'`
    fi
fi

if [ -z "$ARCH" ]; then
    if [ "$WINDOWS" = "1" ]; then
        export ARCH=x86_64
    else
        export ARCH=`uname -m`
    fi
fi
