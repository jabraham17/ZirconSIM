#ifndef ZIRCON_ELF_H_
#define ZIRCON_ELF_H_

#include "mem/memory-image.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <utility>
#include <vector>

namespace elf {

// only implements for 64bit
struct __attribute__((packed)) FileHeader {
    struct __attribute__((packed)) e_ident {
        uint8_t EI_MAG[4];
        uint8_t EI_CLASS;
        uint8_t EI_DATA;
        uint8_t EI_VERSION;
        uint8_t EI_OSABI;
        uint8_t EI_ABIVERSION;
        uint8_t EI_PAD[7];
    } e_ident;
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;

    // 64 bit specific
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;

    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
};
static_assert(sizeof(FileHeader) == 0x40, "FileHeader must be 0x40 bytes");
ssize_t parseFileHeader(std::ifstream&, FileHeader&);
bool validateFileHeader(FileHeader&);

// 64 bit only
struct __attribute__((packed)) ProgramHeader {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
};
static_assert(
    sizeof(ProgramHeader) == 0x38,
    "ProgramHeader must be 0x38 bytes");
ssize_t parseProgramHeader(std::ifstream&, ProgramHeader&);
bool validateProgramHeader(ProgramHeader&);

struct __attribute__((packed)) SectionHeader {
    uint32_t sh_name;
    uint32_t sh_type;
    uint64_t sh_flags;
    uint64_t sh_addr;
    uint64_t sh_offset;
    uint64_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint64_t p_addralign;
    uint64_t p_entsize;
};
static_assert(
    sizeof(SectionHeader) == 0x40,
    "SectionHeader must be 0x40 bytes");
ssize_t parseSectionHeader(std::ifstream&, SectionHeader&);
bool validateSectionHeader(SectionHeader&);

class File {
  private:
    FileHeader fh;
    std::vector<ProgramHeader> phs;
    std::vector<SectionHeader> shs;
    std::ifstream ifs;
    bool valid;

  public:
    File(const std::string& filename) {
        ifs =
            std::ifstream(filename, std::ios_base::binary | std::ios_base::in);
        valid = parse();
    }
    File(std::ifstream ifs) : ifs(std::move(ifs)) { valid = parse(); }
    ~File() { ifs.close(); }
    bool parse();
    bool isValid() { return valid; }

    std::string getSectionHeaderString(uint64_t);

    void buildMemoryImage(mem::MemoryImage&);
    uint64_t getStartAddress();
};

} // namespace elf

#endif
