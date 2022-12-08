#!/usr/bin/env bash

set -e

(set -x && \
    cd hint && \
    ./build.sh $@
)
