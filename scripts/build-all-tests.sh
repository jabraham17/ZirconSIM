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

echo "Building for 'native'"
($ZIRCON_SCRIPTS/build-test.sh native $@) &
if check_toolchain elf; then 
    echo "Building for 'elf'"
    ($ZIRCON_SCRIPTS/build-test.sh elf $@) &
else
    echo "No toolchain for 'elf'"
fi
if check_toolchain elf-debug; then 
    echo "Building for 'elf-debug'"
    ($ZIRCON_SCRIPTS/build-test.sh elf-debug $@) &
else
    echo "No toolchain for 'elf-debug'"
fi
if check_toolchain linux; then 
    echo "Building for 'linux'"
    ($ZIRCON_SCRIPTS/build-test.sh linux $@) &
else
    echo "No toolchain for 'linux'"
fi
if check_toolchain linux-debug; then 
    echo "Building for 'linux-debug'"
    ($ZIRCON_SCRIPTS/build-test.sh linux-debug $@) &
else
    echo "No toolchain for 'linux-debug'"
fi
if check_toolchain musl; then 
    echo "Building for 'musl'"
    ($ZIRCON_SCRIPTS/build-test.sh musl $@) &
else
    echo "No toolchain for 'musl'"
fi
if check_toolchain musl-debug; then 
    echo "Building for 'musl-debug'"
    ($ZIRCON_SCRIPTS/build-test.sh musl-debug $@) &
else
    echo "No toolchain for 'musl-debug'"
fi
wait
