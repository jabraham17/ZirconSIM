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

SOURCE=$1
if [ -z $SOURCE ]; then
    echo "No Source Direcrty Given"
    echo "Correct Usage: $SCRIPT_NAME <gnu-source>"
    exit 1
fi
shift

$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE elf $@
$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE elf-debug $@
$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE linux $@
$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE linux-debug $@
$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE musl $@
$ZIRCON_SCRIPTS/build-toolchain.sh $SOURCE musl-debug $@
