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

source $SCRIPT_DIR/env_vars.sh

FILE1=$1
FILE2=$2
if [[ -z $FILE1 || -z $FILE2 ]]; then
    echo "Need 2 files to compare"
    exit 1
fi

(set -x && diff <($SCRIPT_DIR/objdump.sh -S $FILE1) <($SCRIPT_DIR/objdump.sh -S $FILE2))
