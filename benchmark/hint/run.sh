#!/usr/bin/env bash

set -e

SPIKE=$1
if [ -z $SPIKE ]; then
    echo "Need to specify the spike binary"
    exit 1
fi
shift
ZIRCON=$1
if [ -z $ZIRCON ]; then
    echo "Need to specify the zircon binary"
    exit 1
fi
shift
BENCHMARK=$1
if [ -z $BENCHMARK ]; then
    echo "Need to specify the root benchmark binary directory"
    exit 1
fi
shift
RESULTS=$1
if [ -z $RESULTS ]; then
    echo "Need to specify the root results directory"
    exit 1
fi
shift

mkdir -p $RESULTS/hint
mkdir -p $RESULTS/hint/zircon
mkdir -p $RESULTS/hint/spike

(set -x && cd $RESULTS/hint/zircon && \
    $ZIRCON $BENCHMARK/hint/rv64ima-DOUBLE > d1_out)
(set -x && cd $RESULTS/hint/zircon && \
    $ZIRCON $BENCHMARK/hint/rv64ima-INT > i1_out)
(set -x && cd $RESULTS/hint/zircon && \
    $ZIRCON $BENCHMARK/hint/rv64ima-SHORT > s1_out)
(set -x && cd $RESULTS/hint/zircon && \
    $ZIRCON $BENCHMARK/hint/rv64ima-FLOAT > f1_out)
(set -x && cd $RESULTS/hint/zircon && \
    $ZIRCON $BENCHMARK/hint/rv64ima-LONGLONG > l1_out)

(set -x && cd $RESULTS/hint/spike && \
    $SPIKE $BENCHMARK/hint/rv64ima-DOUBLE > d1_out)
(set -x && cd $RESULTS/hint/spike && \
    $SPIKE $BENCHMARK/hint/rv64ima-INT > i1_out)
(set -x && cd $RESULTS/hint/spike && \
    $SPIKE $BENCHMARK/hint/rv64ima-SHORT > s1_out)
(set -x && cd $RESULTS/hint/spike && \
    $SPIKE $BENCHMARK/hint/rv64ima-FLOAT > f1_out)
(set -x && cd $RESULTS/hint/spike && \
    $SPIKE $BENCHMARK/hint/rv64ima-LONGLONG > l1_out)
