

# TODO LIST

## core features

- load any instructions, not just ELF64 executable
- improve elf support
  - better error checking
  - elf symbols
  - better handling of elf32/elf63, this would allow us to run riscv32 binaries. perhaps as a separate arch
- add permissions to memory, RWX
  - stack should be RW
  - code should be RX
  - heap should be RW
- optimize memory
  - if allocations are next to one another, merge them, similar to optimizing UnionFind
- support threading
  - BIG effort
  - make hart state have a pointer to a single memory image. we can also allocate thread local memory. the shared pointer to memory image controlled via a lock.
- support dynamic linking
  - method 1: load libs in elf loader then emulate as normal
  - method 2: for known libraries (libc), just call into the native x86 library if it exists, if not, fall back to method 1. but this should be preferred as it will be much much faster. even in wasm this is better
- add JIT compiler
  - three versions of JIT both work for static and dynamic
  - method 1: lazy JIT, every time we jump to a new section of code compile, transpile to x86, then do the jump to the native x86 code. save that transpiled x86 code so we don't have to recompile it. this is essentially an optimizing compiler. we can then add heuristics of WHEN to do this. do we do it immediately or wait a couple of times to see if it is hot code?
  - method 2: eager JIT, transpile everything, then jump to start and just execute
  - method 3: conversion, transpile everything and write out a new executable that will run natively. this isn't really JIT, more like a transpiler.
- interactive mode
  - function similar to gdb
  - can use command line controller to kick into interactive after a time period, or just start in interactive
  - single stepping, breakpoints, etcetera
- elf symbols
  - make symbols appear on left column, add to jumps/calls disasm
- flag to enable disasm of instruction aliases like call and ret
- look at binary translation
  - https://ieeexplore.ieee.org/document/671403
- enable a mode that just runs a function with a given set of initial conditions

## refactor

- cleanup instruction specification, CUSTOM instructions are getting messy
- refactor the cpu subdir, inconsistent namespace usage, inconsistent variable naming, better structure
- fix makefile depens for headers and inc
- cleanup exceptions, make more consistent
- refactor riscv stuff/isa stuff to an arch directory outside of the hart
  - select at compile time, but later add runtime support?
  - 

## documentation

- make better readme, current is pitifully inadequate
- specify how to add new instructions, registers, syscalls
- describe the overall arch
- how instructions work
- how registers work
- explain the hart state
- explain the event structure
  - list out the events
- describe how stats work
- 

## tracing

- add full register state tracings
- add ability to efficiently dump memory
- stack tracing
  - trace call stack, function frames
  - add debug info
  - memory trace

## wasm

- make wasm more robust
- improve UI
- add register view
- add register modification
- add ebreak support
- add single stepping
- add pause and resume
- add memory explorer
- 

## instruction sets

- A extension
- Zicsr extension
  - implement csrs
- F extension
- D extension
- Zifencei extension
- C extension
- add proper 32 bit support
- Q extension

## tests

- build a test harness to validate the core itself
  - likely want to fuzz tests to really stress it
- build a reconfigurable test harness for others to use this as their test engine
- perf tests to compare native to simulation, obviously slower but by how much?
