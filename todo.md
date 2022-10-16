

# TODO LIST

## core features

- load any instructions, not just ELF64 executable
- add cmd line args and env args
- improve elf support
  - better error checking
  - elf symbols
- add permissions to memory, RWX
  - stack should be RW
  - code should be RX
  - heap should be RW
- support threading
  - BIG effort

## refactor

- HartState is more accurately described as ThreadState
- cleanup instruction specification, CUSTOM instructions are getting messy

## documentation

- make better readme, current is pitifully inadequate
- specify how to add new instructions, registers, syscalls

## tracing

- more configurable trancing
  - separate files
  - machine readable formats for testing harnesses
- add full register state tracings
- add ability to efficiently dump memory

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
- xBGAS extension
  - this will require some memory system rewrites
- Zicsr extension
  - implement the csrs
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
