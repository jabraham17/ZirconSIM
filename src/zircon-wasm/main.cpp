

#include "cpu/cpu.h"
#include "elf/elf.h"
#include "mem/memory-image.h"
#include "trace/trace.h"
#include <fstream>
#include <iomanip>
#include <iostream>

#include <emscripten.h>
#include <emscripten/bind.h>

void emulate_impl(std::string filename) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return;
    }

    TraceMode traces = TraceMode::INSTRUCTION | TraceMode::REGISTER | TraceMode::MEMORY;
    bool stats = false;

    elf::File f(std::move(is));
    mem::MemoryImage memimg(0xF0000, traces);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    cpu::Hart hart(memimg, traces, stats);
    hart.init();
    hart.execute(start);
}

void emulate(std::string filename) {
    try {
        emulate_impl(filename);
    } catch(const std::exception& e) {
        std::cerr << "Unhandled Exception: " << e.what() << std::endl;
    }
}

EMSCRIPTEN_BINDINGS(ZirconSIM) { emscripten::function("emulate", &emulate); }
