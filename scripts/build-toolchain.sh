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
    echo "Correct Usage: $SCRIPT_NAME <gnu-source> <toolchain>"
    exit 1
fi
shift
TOOLCHAIN=$1
if [ -z $TOOLCHAIN ]; then
    echo "No Toolchain name"
    echo "Correct Usage: $SCRIPT_NAME <gnu-source> <toolchain>"
    exit 1
fi
shift

# make source absolute
case $SOURCE in
    # abs path
    /*) SOURCE=$SOURCE ;;
    # relative path
    *) SOURCE=`pwd`/$SOURCE ;;
esac


BUILD=""
INSTALL=""
TARGET=""
FLAGS=""
if [[ $TOOLCHAIN == "elf" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima
    INSTALL=$ZIRCON_RV64IMA_ELF
    FLAGS="--with-target-cflags=\"$@\" --with-target-cxxflags=\"$@\""
elif [[ $TOOLCHAIN == "elf-debug" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima-debug
    INSTALL=$ZIRCON_RV64IMA_ELF_DEBUG
    FLAGS="--with-target-cflags=\"-g $@\" --with-target-cxxflags=\"-g $@\""
elif [[ $TOOLCHAIN == "linux" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima-linux
    INSTALL=$ZIRCON_RV64IMA_LINUX
    TARGET="linux"
    FLAGS="--with-target-cflags=\"$@\" --with-target-cxxflags=\"$@\""
elif [[ $TOOLCHAIN == "linux-debug" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima-linux-debug
    INSTALL=$ZIRCON_RV64IMA_LINUX_DEBUG
    TARGET="linux"
    FLAGS="--with-target-cflags=\"-g $@\" --with-target-cxxflags=\"-g $@\""
elif [[ $TOOLCHAIN == "musl" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima-musl
    INSTALL=$ZIRCON_RV64IMA_MUSL
    TARGET="musl"
    FLAGS="--with-target-cflags=\"$@\" --with-target-cxxflags=\"$@\""
elif [[ $TOOLCHAIN == "musl-debug" ]]; then
    BUILD=$ZIRCON_BUILD_TOOLCHAINS/rv64ima-musl-debug
    INSTALL=$ZIRCON_RV64IMA_MUSL_DEBUG
    TARGET="musl"
    FLAGS="--with-target-cflags=\"-g $@\" --with-target-cxxflags=\"-g $@\""
else
    echo "Invalid Toolchain"
    exit 1
fi

PARALLEL=`nproc`

mkdir -p $BUILD
mkdir -p $INSTALL

# TODO: add support to specify arch/abi from cmdline
NEWPATH=$INSTALL/bin:$PATH
(set -x && cd $BUILD && PATH=$NEWPATH $SOURCE/configure --prefix=$INSTALL \
    --srcdir=$SOURCE \
    --with-arch=rv64ima \
    --with-abi=lp64 \
    --with-cmodel=medany \
    $FLAGS \
    && PATH=$NEWPATH make $TARGET -j $PARALLEL)




