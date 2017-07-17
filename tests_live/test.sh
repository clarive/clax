#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: <url>"
    exit 255;
fi

URL=$1

function fail {
    echo 'FAILED'
    tail test.log
    exit 255
}

function ok {
    echo 'ok'
}

function run {
    CMD="$1"
    EXPECTED=$2

    echo -n "${CMD}... "

    if $CMD 2>&1 | tee -a test.log | grep "$EXPECTED" > /dev/null && ok; then
        true
    else
        fail
    fi
}

unlink test.log

run "curl -kv $URL/" "200 OK"

run "curl -kv $URL/ping" "pong"

run "curl -kv $URL/tree/clax.c" "200 OK"

run "curl -kv -F file=@../clax.c $URL/tree/" "200 OK"

run "curl -kv -d 'echo hi' $URL/command" "hi"
