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

($SCRIPT_DIR/build_test.sh native $@) &
($SCRIPT_DIR/build_test.sh elf $@) &
($SCRIPT_DIR/build_test.sh elf-debug $@) &
($SCRIPT_DIR/build_test.sh linux $@) &
($SCRIPT_DIR/build_test.sh linux-debug $@) &
($SCRIPT_DIR/build_test.sh musl $@) &
($SCRIPT_DIR/build_test.sh musl-debug $@) &
wait
