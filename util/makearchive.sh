#!/bin/bash

#set -x

VERSION=$(tr -d "\012" < VERSION)

DIST="clax_$VERSION";
ARCHIVE="$DIST.tar"

git archive --format=tar "--prefix=$DIST/" HEAD > "$ARCHIVE"
tar -rf "$ARCHIVE" --transform "s,^,$DIST/," clax_version.h VERSION
gzip "$ARCHIVE"

echo
echo "DONE"
echo
echo "$ARCHIVE.gz"
