

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

struct MemoryException : public std::runtime_error {
    MemoryException() : std::runtime_error("Memory Exception") {}
    MemoryException(std::string message)
        : std::runtime_error("Memory Exception: " + message) {}
    MemoryException(std::string type, std::string message)
        : std::runtime_error("Memory Exception[" + type + "]: " + message) {}
};

struct ReallocationMemoryException : public MemoryException {
    types::Address addr;
    types::UnsignedInteger size;
    ReallocationMemoryException(types::Address addr, types::UnsignedInteger size = 0)
        : MemoryException("Reallocation", ""), addr(addr), size(size) {}
    const char* what() const noexcept {
        std::stringstream ss;
        ss << MemoryException::what() << "\n";
        ss << "attempted to reallocate 0x" << std::hex << addr;
        if(size != 0) {
            ss << " of size 0x" << std::hex << addr;
        }
        return strdup(ss.str().c_str());
    }
};
// struct OutOfMemoryException : public MemoryException {
//     OutOfMemoryException(std::string message = "") :
//     MemoryException("OutOfMemory", message) {}
// };
struct OutOfBoundsException : public MemoryException {
    types::Address addr;
    types::Address range_lower;
    types::Address range_upper;
    OutOfBoundsException(
        types::Address addr,
        types::Address range_lower = 0,
        types::Address range_upper = 0)
        : MemoryException("OutOfBounds", ""), addr(addr),
          range_lower(range_lower), range_upper(range_upper) {}
    const char* what() const noexcept {
        std::stringstream ss;
        ss << MemoryException::what() << "\n";
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
        types::Address address;
        types::UnsignedInteger size;
        uint8_t* buffer;
        MemoryRegion(types::Address address, types::UnsignedInteger size, uint8_t* buffer)
            : address(address), size(size), buffer(buffer) {}

        const uint8_t* raw(types::Address addr) const {
            if(addr >= address && addr < address + size)
                return buffer + (addr - address);
            else throw OutOfBoundsException(addr, address, address + size);
        }
        uint8_t* raw(types::Address addr) {
            return const_cast<uint8_t*>(std::as_const(*this).raw(addr));
        }
        uint8_t& byte(types::Address addr) { return *((uint8_t*)this->raw(addr)); }
        uint16_t& halfword(types::Address addr) {
            return *((uint16_t*)this->raw(addr));
        }
        uint32_t& word(types::Address addr) { return *((uint32_t*)this->raw(addr)); }
        uint64_t& doubleword(types::Address addr) {
            return *((uint64_t*)this->raw(addr));
        }
    };

    std::vector<MemoryRegion> memory_map;

    // Subsystem: mem
    // Description: Fires when memory is read
    // Parameters: (address, value read, n bytes)
    event::Event<types::Address, uint64_t, size_t> event_read;
    // Subsystem: mem
    // Description: Fires when memory is written
    // Parameters: (address, value written, old value, n bytes)
    event::Event<types::Address, uint64_t, uint64_t, size_t> event_write;
    // Subsystem: mem
    // Description: Currently unimplemented
    // Parameters:
    event::Event<uint64_t, uint64_t> event_exception;
    // Subsystem: mem
    // Description: Fires when memory is allocated
    // Parameters: (base address, allocation size)
    event::Event<types::Address, uint64_t> event_allocation;

    MemoryRegion& allocateMemoryRegion(types::Address addr, uint64_t size = 8) {
        uint8_t* ptr = (uint8_t*)malloc(sizeof(*ptr) * size);
        MemoryRegion mr(addr, size, ptr);
        memory_map.push_back(mr);
        return memory_map.back();
    }

    const MemoryRegion* getMemoryRegion(types::Address addr) const {
        for(auto& mr : memory_map) {
            if(addr >= mr.address && addr < mr.address + mr.size) {
                return &mr;
            }
        }
        return nullptr;
    }
    MemoryRegion* getMemoryRegion(types::Address addr) {
        return const_cast<MemoryRegion*>(
            std::as_const(*this).getMemoryRegion(addr));
    }

    template <typename T> struct MemoryCellProxy {
      private:
        MemoryImage* mi;
        types::Address addr;
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
        MemoryCellProxy(MemoryImage* mi, types::Address addr) : mi(mi), addr(addr) {}
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
    MemoryImage() = default;

    void allocate(types::Address addr, uint64_t size) {
        if(size == 0) return;
        event_allocation(addr, size);
        if(getMemoryRegion(addr)) {
            throw ReallocationMemoryException(addr, size);
        }
        allocateMemoryRegion(addr, size);
    }

    MemoryCellProxy<uint8_t> byte(types::Address addr) {
        return MemoryCellProxy<uint8_t>(this, addr);
    }
    MemoryCellProxy<uint16_t> halfword(types::Address addr) {
        return MemoryCellProxy<uint16_t>(this, addr);
    }
    MemoryCellProxy<uint32_t> word(types::Address addr) {
        return MemoryCellProxy<uint32_t>(this, addr);
    }
    MemoryCellProxy<uint64_t> doubleword(types::Address addr) {
        return MemoryCellProxy<uint64_t>(this, addr);
    }
    const uint8_t* raw(types::Address addr) const {
        auto mr = getMemoryRegion(addr);
        if(mr) {
            return mr->raw(addr);
        } else return nullptr;
    }
    uint8_t* raw(types::Address addr) {
        return const_cast<uint8_t*>(std::as_const(*this).raw(addr));
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
