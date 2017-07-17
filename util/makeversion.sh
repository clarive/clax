#!/bin/sh

if which git > /dev/null && [ -d ".git" ]; then
    GIT_VERSION="$(date +%Y%m%d)-$(git rev-parse --short HEAD)"

    echo "$GIT_VERSION" > VERSION
fi

if [ ! -f 'VERSION' ]; then
    echo 'Error: VERSION file does not exist';
    exit 255
fi

VERSION=$(tr -d "\012" < VERSION)

sed "s/CLAX_VERSION \".*\"/CLAX_VERSION \"$VERSION\"/" clax_version.h.template > clax_version.h

. util/detect-os.sh

echo "$OS" > OS
echo "$ARCH" > ARCH

sed "s/CLAX_OS \".*\"/CLAX_OS \"$OS\"/" clax_version.h > clax_version.h.2
mv clax_version.h.2 clax_version.h
sed "s/CLAX_ARCH \".*\"/CLAX_ARCH \"$ARCH\"/" clax_version.h > clax_version.h.2
mv clax_version.h.2 clax_version.h
