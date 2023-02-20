#!/usr/bin/env bash

# set -e
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

export CF=clang-format
# FIXME: required until LLVM 16 when fix for .inc hits the release
export CF=$ZIRCON_HOME/../llvm/llvm-project/build/bin/clang-format
export STYLE=$ZIRCON_HOME/.clang-format

check_format() {
    file=$1
    diff <($CF --Werror --style=file:$STYLE $file) $file >/dev/null 2>&1
}
export -f check_format
FILES=("*.c" "*.cpp" "*.cc" "*.h" "*.hpp" "*.inc")
for s in "${FILES[@]}"; do
    (find $ZIRCON_HOME/src -ipath $s -type f -exec bash -c 'check_format "$0" && test $? -eq 0 || echo $(echo $0 | sed "s;$ZIRCON_HOME/;;g") needs to be formatted' {} \; )
done
