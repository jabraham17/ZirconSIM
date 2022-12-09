

#include "common/format.h"
#include "cpu/cpu.h"
#include "cpu/isa/inst.h"
#include "elf/elf.h"
#include "mem/memory-image.h"

#include <emscripten.h>
#include <emscripten/bind.h>
#include <fstream>
#include <iomanip>
#include <iostream>

void emulate_impl(std::string filename) {
    std::ifstream is(filename, std::ios::binary);
    if(!is) {
        std::cerr << "Failed to open '" << filename << "'" << std::endl;
        return;
    }

    elf::File f(std::move(is));
    mem::MemoryImage memimg;
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    cpu::Hart hart(memimg);

    std::ostream* inst_log = &std::cout;
    std::ostream* reg_log = &std::cout;

    hart.addBeforeExecuteListener([inst_log](cpu::HartState& hs) {
        auto inst = hs().getInstWord();
        *inst_log << "PC[" << common::Format::doubleword << hs().pc
                  << "] = " << common::Format::word << inst << "; "
                  << isa::inst::disassemble(inst, hs().pc) << std::endl;
    });
    hart.addRegisterReadListener(
        [reg_log](std::string classname, uint64_t idx, uint64_t value) {
            *reg_log << "RD " << classname << "[" << common::Format::dec << idx
                     << "] = " << common::Format::doubleword << (uint64_t)value
                     << std::endl;
        });
    hart.addRegisterWriteListener([reg_log](
                                      std::string classname,
                                      uint64_t idx,
                                      uint64_t value,
                                      uint64_t oldvalue) {
        *reg_log << "WR " << classname << "[" << common::Format::dec << idx
                 << "] = " << common::Format::doubleword << (uint64_t)value
                 << "; OLD VALUE = " << common::Format::doubleword << oldvalue
                 << std::endl;
    });

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
