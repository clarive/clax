#!/bin/bash

BRANCH=`git rev-parse --abbrev-ref HEAD`
SHA=`git rev-parse --short HEAD`

DIST="clax-$BRANCH-$SHA";
ARCHIVE="$DIST.tar.gz"

git archive --format=tar --prefix=$DIST/ HEAD | gzip > $ARCHIVE

echo
echo "DONE"
echo
echo $ARCHIVE
