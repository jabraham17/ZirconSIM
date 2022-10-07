

#include "cpu/cpu.h"
#include "elf/elf.h"
#include "memimg/memimg.h"
#include <fstream>
#include <iomanip>
#include <iostream>

int main(int argc, char** argv) {

    if(argc <= 1) return 1;
    std::string filename = argv[1];
    std::ifstream is(filename, std::ios::binary);
    if(!is) return 1;

    elf::File f(std::move(is));
    memimg::Memory mem(0x80000);
    f.buildMemIMG(mem);
    auto start = f.getStartAddress();

    cpu::Hart64 hart(std::move(mem));
    hart.execute(start);

    // Hart64State hs;
    // hs.rf.dump(std::cout);
    // auto I = (Instructions::getInstruction(0x000017b7));
    // I.execute(hs);
    // hs.rf.dump(std::cout);

    // for(int i = 0; i < 2; i++) {

    //     // uint32_t word = mem.at(start) | mem.at(start + 1) << 8|
    //     //        mem.at(start + 2) << 16 |
    //     //        mem.at(start + 3) << 24;
    //         //    uint32_t word2 = mem.word(start);
    //         uint32_t word = mem.word(start);
    // std::cout << std::hex << start << "\n";
    //     std::cout << "0x" << std::setfill('0') << std::setw(8) << std::right
    //     << std::hex << word << ": ";
    //     // std::cout << (int)(word == word2);
    //         // Instructions::Instruction test(word);
    //     // std::cout << "0x" << std::hex << test.getOpcode() << "\n";
    //     // std::cout << "0x" << std::hex << test.getFunct3() << "\n";
    //     // std::cout << "0x" << std::hex << test.getFunct7() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getOpcode() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getFunct3() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getFunct7() << "\n";

    //     auto* I = Instructions::getInstruction(word);
    //     if(I) {I->print(std::cout);std::cout << "\n";}
    //     else std::cout << "unknown\n";
    //     start += 4;
    // }

    // is.seekg(0, is.end);
    // auto length = is.tellg();
    // is.seekg(0, is.beg);

    // uint8_t* instruction_memory = new uint8_t[length];

    // is.read((char*)instruction_memory, length);

    // uint32_t word;
    // for(auto i = 0; i < length; i += 4) {
    //     word = instruction_memory[i] | instruction_memory[i + 1] << 8|
    //            instruction_memory[i + 2] << 16 |
    //            instruction_memory[i + 3] << 24;
    //     std::cout << "0x" << std::setfill('0') << std::setw(8) << std::right
    //     << std::hex << word << ": ";

    //     // Instructions::Instruction test(word);
    //     // std::cout << "0x" << std::hex << test.getOpcode() << "\n";
    //     // std::cout << "0x" << std::hex << test.getFunct3() << "\n";
    //     // std::cout << "0x" << std::hex << test.getFunct7() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getOpcode() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getFunct3() << "\n";
    //     // std::cout << "0x" << std::hex <<
    //     Instructions::isa_32i[3]->getFunct7() << "\n";

    //     auto* I = Instructions::getInstruction(word);
    //     if(I) {I->print(std::cout);std::cout << "\n";}
    //     else std::cout << "unknown\n";
    // }

    // is.close();

    return 0;
}
