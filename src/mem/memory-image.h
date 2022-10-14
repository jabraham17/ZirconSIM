

#ifndef ZIRCON_MEM_MEMORY_IMAGE_H_
#define ZIRCON_MEM_MEMORY_IMAGE_H_

#include "trace/trace.h"
#include <cstdint>
#include <iostream>
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

class MemoryImage {

  private:
    struct MemoryRegion {
        uint64_t address;
        uint64_t size;
        uint8_t* buffer;
        MemoryRegion(uint64_t address, uint64_t size, uint8_t* buffer)
            : address(address), size(size), buffer(buffer) {}

        uint8_t* raw(uint64_t addr) {
            if(addr >= address && addr < address + size)
                return buffer + (addr - address);
            else throw OutOfBoundsException();
        }
        // uint8_t& at(uint64_t addr) { return *((uint8_t*)this->get(addr)); }
        uint8_t& byte(uint64_t addr) { return *((uint8_t*)this->raw(addr)); }
        uint16_t& halfword(uint64_t addr) {
            return *((uint16_t*)this->raw(addr));
        }
        uint32_t& word(uint64_t addr) { return *((uint32_t*)this->raw(addr)); }
        uint64_t& doubleword(uint64_t addr) {
            return *((uint64_t*)this->raw(addr));
        }
    };

    std::vector<MemoryRegion> memory_map;
    std::unique_ptr<uint8_t[]> memory;
    uint8_t* memory_ptr;
    size_t mem_size;

    Trace trace_mem;

    MemoryRegion& allocateMemoryRegion(uint64_t addr, uint64_t size = 8) {
        if(memory_ptr - memory.get() + size >= mem_size) {
            throw OutOfMemoryException();
        }

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

    template <typename T> struct MemoryCellProxy {
      private:
        MemoryImage* mi;
        uint64_t addr;
        friend MemoryImage;
        auto constexpr getPrintFormatter() {
            if(std::is_same<T, uint8_t>::value) return Trace::byte;
            else if(std::is_same<T, uint16_t>::value) return Trace::halfword;
            else if(std::is_same<T, uint32_t>::value) return Trace::word;
            else if(std::is_same<T, uint64_t>::value) return Trace::doubleword;
            return Trace::doubleword;
        }
        MemoryRegion& mr() { return mi->getMemoryRegion(addr); }

      public:
        T read();
        void write(T v);
        MemoryCellProxy(MemoryImage* mi, uint64_t addr) : mi(mi), addr(addr) {}
        operator T() {
            mi->trace_mem << "RD MEM[" << Trace::doubleword << addr << "]"
                          << Trace::flush;
            T v = read();
            mi->trace_mem << " = " << getPrintFormatter() << (uint64_t)v
                          << std::endl;
            return v;
        }
        MemoryCellProxy<T>& operator=(T v) {
            mi->trace_mem << "WR MEM[" << Trace::doubleword << addr << "]"
                          << Trace::flush;
            T old_value = read();
            write(v);
            mi->trace_mem << " = " << getPrintFormatter() << (uint64_t)v
                          << "; OLD VALUE = " << getPrintFormatter()
                          << (uint64_t)old_value << std::endl;
            return *this;
        }
    };

  public:
    MemoryImage(size_t size = 0x1000, TraceMode tm = TraceMode::NONE)
        : mem_size(size), trace_mem("MEMORY TRACE") {
        memory = std::make_unique<uint8_t[]>(mem_size);
        memory_ptr = memory.get();
        trace_mem.setState(tm & TraceMode::MEMORY);
    }

    void allocate(uint64_t addr, uint64_t size) {
        trace_mem << "ALLOCATE[" << Trace::doubleword << addr << " - ";
        trace_mem << Trace::doubleword << addr + size << "]";
        trace_mem << std::endl;
        allocateMemoryRegion(addr, size);
    }

    MemoryCellProxy<uint8_t> byte(uint64_t addr) {
        return MemoryCellProxy<uint8_t>(this, addr);
    }
    MemoryCellProxy<uint16_t> halfword(uint64_t addr) {
        return MemoryCellProxy<uint16_t>(this, addr);
    }
    MemoryCellProxy<uint32_t> word(uint64_t addr) {
        return MemoryCellProxy<uint32_t>(this, addr);
    }
    MemoryCellProxy<uint64_t> doubleword(uint64_t addr) {
        return MemoryCellProxy<uint64_t>(this, addr);
    }
    uint8_t* raw(uint64_t addr) {
        auto mr = getMemoryRegion(addr);
        return mr.raw(addr);
    }
};

} // namespace mem

#endif
