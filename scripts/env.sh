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


export ZIRCON_HOME=$SCRIPT_DIR/..
export ZIRCON_SCRIPTS=$ZIRCON_HOME/scripts
export ZIRCON_TEST=$ZIRCON_HOME/test
export ZIRCON_TEST_BUILD=$ZIRCON_TEST/build
export ZIRCON_BENCHMARK=$ZIRCON_HOME/benchmark
export ZIRCON_TOOLCHAINS=$ZIRCON_HOME/toolchains
export ZIRCON_BUILD_TOOLCHAINS=$ZIRCON_TOOLCHAINS/build

export ZIRCON_RV64IMA_ELF=$ZIRCON_TOOLCHAINS/rv64ima
export ZIRCON_RV64IMA_ELF_DEBUG=$ZIRCON_TOOLCHAINS/rv64ima-debug

export ZIRCON_RV64IMA_LINUX=$ZIRCON_TOOLCHAINS/rv64ima-linux
export ZIRCON_RV64IMA_LINUX_DEBUG=$ZIRCON_TOOLCHAINS/rv64ima-linux-debug

export ZIRCON_RV64IMA_MUSL=$ZIRCON_TOOLCHAINS/rv64ima-musl
export ZIRCON_RV64IMA_MUSL_DEBUG=$ZIRCON_TOOLCHAINS/rv64ima-musl-debug


