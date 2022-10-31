# Zircon Documentation

The primary purpose of this documentation is to guide both usage and future development of Zircon.

## Building

Building Zircon requires a C++14 toolchain on x86-64 Linux.
No other build platforms are currently supported.

Zircon requires a very small number of libraries, the eventual goal is that the implementation is entirely free standing and requires only a C++ toolchain.
However to simplify some of the early development, there are some libraries required.

Required Libraries
- popt: command line argument parsing
  - `dnf install popt-devel`

If building for the web, Emscripten is the required toolchain.
The toolchain is available [here](https://emscripten.org/docs/getting_started/downloads.html).

### Native

To build the native command line tool, the default `Makefile` will build everything required.

`make`

This will put the built binary in `./build/bin/zircon`.

Debug builds can be automatically built.

`make DEBUG=1`

It is recommended to build to a different directory than the default, this can be done as follows

`make DEBUG=1 BUILD=build-debug`

The binary will now be outputted to `./build-debug/bin/zircon`.

### WASM

To build for the web, specify `WASM=1` in the `make` command.
It is also recommend to do this in a separate build folder.

`make WASM=1 BUILD=build-wasm`

The resulting site is outputted to `./build-wasm/zircon-site`.
A sample HTTP server can be run as follows.

`(cd build-wasm/zircon-site/ && python3 -m http.server)`

This uses Emscripten and requires `em++` to be on the `PATH`.
See the Emscripten [installation directions](https://emscripten.org/docs/getting_started/downloads.html).

## Usage

Zircon is primarily a command line based simulator tool.
It can serve as a direct simulator for Linux or baremetal executables for RISC-V 64 bit running on a host x86 Linux system.
Future goals will be to implement some binary translation to enable a transpiler for RISC-V to x86 and the creation of a JIT engine.

A full description of the command line interface is [here](cli.md). 
The command line tool provides a number of out-of-the-box tools to run, debug, and inspect RISC-V binaries.

The web front end is highly experimental, but should run fine in most browser.
View the documentation [here](web.md).

## Development

