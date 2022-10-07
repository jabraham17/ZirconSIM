

#include "cpu/cpu.h"
#include "elf/elf.h"
#include "mem/memory-image.h"
#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {

    if(argc <= 1) return 1;
    std::string filename = argv[1];
    std::ifstream is(filename, std::ios::binary);
    if(!is) return 1;

    elf::File f(std::move(is));
    mem::MemoryImage memimg(0x80000);
    f.buildMemoryImage(memimg);
    auto start = f.getStartAddress();

    cpu::Hart64 hart(std::move(memimg));
    hart.execute(start);

    return 0;
}
