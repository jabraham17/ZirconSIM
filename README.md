# ZirconSIM

![Zircon Icon](img/icon.svg)

ZirconSIM (also just Zircon) is a RISC-V ISA simulator designed to support rapid software development.
Zircon can load ELF64 RISC-V executables compiled for a variety of platforms and run them with reasonable efficiency.
Zircon was created with developers in mind and supports a large number of operations to improve the debugging of RISC-V code.
This supports software development in the vacuum of available hardware for RISC-V, as well as providing inspection that would not be available on hardware.

## Building

Building Zircon requires a C++17 toolchain on x86-64 Linux.
No other build platforms are currently supported.
Zircon requires no external libraries besides a C++ toolchain.

Building the tests requires a RISC-V toolchain.
The primary supported toolchain is the [riscv-gnu-toolchain](https://github.com/riscv-collab/riscv-gnu-toolchain).
This provides baremetal and Linux support for building RISC-V ELF64 binaries.
Scripts are provided to build this automatically.

### Native

To build the native command line tool, the default `Makefile` will build everything required.

`make`

This will put the built binary in `./build/bin/zircon`.

Debug builds can be automatically built.

`make DEBUG=1`

It is recommended to build to a different directory than the default, this can be done as follows

`make DEBUG=1 BUILD=build-debug`

The binary will now be outputted to `./build-debug/bin/zircon`.

If making changes to Zircon, we currently recommend you build with `make -B`, as changes to certain headers and `.inc` files will not trigger the correct rebuilds.
This is an open bug and will be fixed in the future.

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

To run a program (or the test suite), you need to build the source code for RISC-V.
The easiest way to do this is to build and install a RISC-V toolchain.
There are a number of ways to do this.
If you are just interested in trying out Zircon and haven't done this before, the easiest way is to clone the `gcc` toolchain with `$ git clone --recursive https://github.com/riscv/riscv-gnu-toolchain` and use Zircon's helper scripts to build it.

If you have cloned it into the Zircon home directory with the above command, you can build a toolchain from the Zircon home directory directory as `./scripts/build-toolchain.sh riscv-gnu-toolchain elf`.
This will build the baremetal toolchain into `toolchains/rv64ima`.
Other toolchains are available, see the build script (or use `./scripts/build-all-toolchains.sh` to build them all).

If you are comfortable with creating cross compiling toolchains for RISC-V you can use your own.
Note that Zircon currently only supports `RV64IMA` with `lp64` ABI.

**Note:** support for the `A` ISA extension is limited.

Successfully built executables can be ran as `./build/bin/zircon path/to/executable`.
If you want to try out some of the testing programs, they can be built automatically using the `./scripts/build-test.sh` or `./scripts/build-all-tests.sh` scripts.

A test suite is in development, once it is more fully developed documentation will be added on how to run it.
