#!/usr/bin/env bash

set -e
realpath() {
  OURPWD=$PWD
  cd "$(dirname "$1")"
  LINK=$(readlink "$(basename "$1")")
  while [ "$LINK" ]; do
    cd "$(dirname "$LINK")"
    LINK=$(readlink "$(basename "$1")")
  done
  REALPATH="$PWD/$(basename "$1")"
  cd "$OURPWD"
  echo "$REALPATH"
}
SCRIPT_NAME=
if [ -z $BASH_SOURCE ]; then
    SCRIPT_NAME=$0
else
    SCRIPT_NAME=${BASH_SOURCE[0]}
fi
SCRIPT_DIR=$(realpath "$(dirname "$SCRIPT_NAME")")
source $SCRIPT_DIR/env.sh

PYTHON=`which python3`
if [ $? -ne 0 ]; then
    PYTHON=`which python`
    if [ $? -ne 0]; then
        echo "Missing Python 3 installation: requires 3.9+"
        exit 1
    fi
fi
if [ `$PYTHON -V | sed -r 's/Python\s([0-9]+)\.([0-9]+).*/\1/'` -ne 3 ]; then
    echo "Missing Python 3 installation: requires 3.9+"
    exit 1
fi
if [ `$PYTHON -V | sed -r 's/Python\s([0-9]+)\.([0-9]+).*/\2/'` -lt 8 ]; then
    echo "Missing Python 3 installation: requires 3.9+"
    exit 1
fi

TEST_SCRIPT=$SCRIPT_DIR/../tests/driver/driver.py

$PYTHON $TEST_SCRIPT $@
