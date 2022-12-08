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

($ZIRCON_SCRIPTS/build_test.sh native $@) &
($ZIRCON_SCRIPTS/build_test.sh elf $@) &
($ZIRCON_SCRIPTS/build_test.sh elf-debug $@) &
($ZIRCON_SCRIPTS/build_test.sh linux $@) &
($ZIRCON_SCRIPTS/build_test.sh linux-debug $@) &
($ZIRCON_SCRIPTS/build_test.sh musl $@) &
($ZIRCON_SCRIPTS/build_test.sh musl-debug $@) &
wait
