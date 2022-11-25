

#ifndef ZIRCON_MEM_MEMORY_IMAGE_H_
#define ZIRCON_MEM_MEMORY_IMAGE_H_

#include "event/event.h"
#include <cstdint>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace mem {

struct MemoryException : public std::exception {
    const char* what() const noexcept { return "Memory Exception"; }
};
struct OutOfMemoryException : public MemoryException {
    const char* what() const noexcept { return "Out of Memory"; }
};
struct OutOfBoundsException : public MemoryException {
    uint64_t addr;
    uint64_t range_lower;
    uint64_t range_upper;
    OutOfBoundsException(
        uint64_t addr,
        uint64_t range_lower = 0,
        uint64_t range_upper = 0)
        : addr(addr), range_lower(range_lower), range_upper(range_upper) {}
    const char* what() const noexcept {
        std::stringstream ss;
        ss << "Out of Bounds Access: ";
        if(range_lower == 0 && range_upper == 0) {
            ss << "unknown addr 0x" << std::hex << addr;
        } else {
            ss << "addr 0x" << std::hex << addr << " not in range [0x"
               << std::hex << range_lower << "-0x" << std::hex << range_upper
               << "]";
        }
        return strdup(ss.str().c_str());
    }
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
            else throw OutOfBoundsException(addr, address, address + size);
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
    // std::unique_ptr<uint8_t[]> memory;
    // uint8_t* memory_ptr;
    size_t mem_size;

    // Subsystem: mem
    // Description: Fires when memory is read
    // Parameters: (address, value read, n bytes)
    event::Event<uint64_t, uint64_t, size_t> event_read;
    // Subsystem: mem
    // Description: Fires when memory is written
    // Parameters: (address, value written, old value, n bytes)
    event::Event<uint64_t, uint64_t, uint64_t, size_t> event_write;
    // Subsystem: mem
    // Description: Currently unimplemented
    // Parameters:
    event::Event<uint64_t, uint64_t> event_exception;
    // Subsystem: mem
    // Description: Fires when memory is allocated
    // Parameters: (base address, allocation size)
    event::Event<uint64_t, uint64_t> event_allocation;

    MemoryRegion& allocateMemoryRegion(uint64_t addr, uint64_t size = 8) {
        // if(memory_ptr - memory.get() + size >= mem_size) {
        //     throw OutOfMemoryException();
        // }

        // auto ptr = memory_ptr;
        uint8_t* ptr = (uint8_t*)malloc(sizeof(*ptr)*size);
        MemoryRegion mr(addr, size, ptr);
        memory_map.push_back(mr);
        // memory_ptr += size;
        return memory_map.back();
    }

    MemoryRegion* getMemoryRegion(uint64_t addr) {
        for(auto& mr : memory_map) {
            if(addr >= mr.address && addr < mr.address + mr.size) {
                return &mr;
            }
        }
        return nullptr;
    }

    template <typename T> struct MemoryCellProxy {
      private:
        MemoryImage* mi;
        uint64_t addr;
        friend MemoryImage;
        auto constexpr getSize() {
            if(std::is_same<T, uint8_t>::value) return 1;
            else if(std::is_same<T, uint16_t>::value) return 2;
            else if(std::is_same<T, uint32_t>::value) return 4;
            else if(std::is_same<T, uint64_t>::value) return 8;
            return 8;
        }
        MemoryRegion& mr() {
            auto mr_ptr = mi->getMemoryRegion(addr);
            if(mr_ptr) return *mr_ptr;
            else throw OutOfBoundsException(addr);
        }

      public:
        T read();
        void write(T v);
        MemoryCellProxy(MemoryImage* mi, uint64_t addr) : mi(mi), addr(addr) {}
        operator T() {
            T v = read();
            mi->event_read(addr, v, getSize());
            return v;
        }
        MemoryCellProxy<T>& operator=(T v) {
            T old_value = read();
            write(v);
            mi->event_write(addr, v, old_value, getSize());
            return *this;
        }
    };

  public:
    MemoryImage(size_t size = 0x1000) : mem_size(size) {
        // memory = std::make_unique<uint8_t[]>(mem_size);
        // memory_ptr = memory.get();
    }

    void allocate(uint64_t addr, uint64_t size) {
        event_allocation(addr, size);
        if(size == 0) return;
        if(getMemoryRegion(addr)) {
            throw MemoryException();
        }
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
    uint8_t* raw(uint64_t addr, bool print = false) {
        auto mr = getMemoryRegion(addr);
        if(mr) {
            if(print) {
            std::cout << "Memory region start: " << std::hex << mr->address << "\n";
            std::cout << "Memory region length: " << std::hex << mr->size << "\n";
            }
            return mr->raw(addr);}
        else return nullptr;
    }
    template <typename T> void addReadListener(T&& arg) {
        event_read.addListener(std::forward<T>(arg));
    }
    template <typename T> void addWriteListener(T&& arg) {
        event_write.addListener(std::forward<T>(arg));
    }
    template <typename T> void addAllocationListener(T&& arg) {
        event_allocation.addListener(std::forward<T>(arg));
    }
};

} // namespace mem

#endif
