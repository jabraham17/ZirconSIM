

#ifndef ZIRCON_MEMIMG_H_
#define ZIRCON_MEMIMG_H_

#include <cstdint>
#include <map>
#include <memory>
#include <utility>
#include <vector>

namespace mem {

struct MemoryException : public std::exception {
    const char* what() const throw() { return "Memory Exception"; }
};
struct OutOfMemoryException : public MemoryException {
    const char* what() const throw() { return "Out of Memory"; }
};
struct OutOfBoundsException : public MemoryException {
    const char* what() const throw() { return "Out of Bounds Access"; }
};

struct MemoryRegion {
    uint64_t address;
    uint64_t size;
    uint8_t* buffer;
    MemoryRegion(uint64_t address, uint64_t size, uint8_t* buffer)
        : address(address), size(size), buffer(buffer) {}

    uint8_t* get(uint64_t addr) {
        if(addr >= address && addr < address + size)
            return buffer + (addr - address);
        else throw OutOfBoundsException();
    }
    uint8_t& at(uint64_t addr) { return *((uint8_t*)this->get(addr)); }
    uint8_t& byte(uint64_t addr) { return *((uint8_t*)this->get(addr)); }
    uint16_t& halfword(uint64_t addr) { return *((uint16_t*)this->get(addr)); }
    uint32_t& word(uint64_t addr) { return *((uint32_t*)this->get(addr)); }
    uint64_t& doubleword(uint64_t addr) {
        return *((uint64_t*)this->get(addr));
    }
};

class MemoryImage {

  private:
    std::vector<MemoryRegion> memory_map;
    std::unique_ptr<uint8_t[]> memory;
    uint8_t* memory_ptr;
    size_t mem_size;

    MemoryRegion& allocateMemoryRegion(uint64_t addr, uint64_t size = 8) {
        if(memory_ptr - memory.get() + size >= mem_size)
            throw OutOfMemoryException();

        auto ptr = memory_ptr;
        MemoryRegion mr(addr, size, ptr);
        memory_map.push_back(mr);
        memory_ptr += size;
        return memory_map.back();
    }

    MemoryRegion& getMemoryRegion(uint64_t addr) {
        for(auto& mr : memory_map) {
            if(addr >= mr.address && addr < mr.address + mr.size) {
                return mr;
            }
        }
        throw OutOfBoundsException();
    }

  public:
    MemoryImage(size_t size = 0x1000) : mem_size(size) {
        memory = std::make_unique<uint8_t[]>(mem_size);
        memory_ptr = memory.get();
    }

    void allocate(uint64_t addr, uint64_t size) {
        allocateMemoryRegion(addr, size);
    }

    uint8_t* get(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.get(addr);
    }
    uint8_t& at(uint64_t addr) { return this->byte(addr); }
    uint8_t& byte(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.byte(addr);
    }
    uint16_t& halfword(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.halfword(addr);
    }
    uint32_t& word(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.word(addr);
    }
    uint64_t& doubleword(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.doubleword(addr);
    }
};

} // namespace mem

#endif
