

#include "cpu/cpu.h"
#include "elf/elf.h"
#include "mem/memory-image.h"
#include "trace/trace.h"
#include <fstream>
#include <iomanip>
#include <iostream>

#include <emscripten.h>
#include <emscripten/bind.h>


void emulate(std::string filename) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return;
    }

    TraceMode traces = TraceMode::NONE;
    bool stats = false;

    elf::File f(std::move(is));
    mem::MemoryImage memimg(0x80000, traces);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    cpu::Hart hart(memimg, traces, stats);
    hart.init();
    hart.execute(start);
}

EMSCRIPTEN_BINDINGS(ZirconSIM) { emscripten::function("emulate", &emulate); }
