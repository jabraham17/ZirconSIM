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

ROOT_DIR=$SCRIPT_DIR/..


REGEX=".*// Subsystem:\s*(.*)\n.*// Description:\s*(.*)\n.*// Parameters:\s*(.*)\n.*$ROOT_DIR/(.*):\s*event::Event<(.*)>\s*(.*);"

TABLEFMT="| %10s | %20s | %50s | %60s | %25s |"
printf "$TABLEFMT\n" "Subsystem" "Name" "Description" "Callback Parameters" "Location"
printf "$TABLEFMT\n" "---" "---" "---" "---" "---"
grep -B 3 -rn --no-group-separator $ROOT_DIR/src -e "event::Event<" \
| perl -0777 -pe "s@$REGEX@sprintf \"$TABLEFMT\", \$1, \$6, \$2, \$3, \$4 @egm"

# | perl -pe "s%^$ROOT_DIR/(.*):\s*event::Event<(.*)>\s*(.*);%sprintf \"| \%25s | \%20s | \%50s | \%41s |\", \$1, \$3, \"\", \$2 %eg" \


