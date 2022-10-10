#include "memory-image.h"

template <> uint8_t mem::MemoryImage::MemoryCellProxy<uint8_t>::read() {
    return mr().byte(addr);
}
template <> uint16_t mem::MemoryImage::MemoryCellProxy<uint16_t>::read() {
    return mr().halfword(addr);
}
template <> uint32_t mem::MemoryImage::MemoryCellProxy<uint32_t>::read() {
    return mr().word(addr);
}
template <> uint64_t mem::MemoryImage::MemoryCellProxy<uint64_t>::read() {
    return mr().doubleword(addr);
}

template <> void mem::MemoryImage::MemoryCellProxy<uint8_t>::write(uint8_t v) {
    mr().byte(addr) = v;
}
template <> void mem::MemoryImage::MemoryCellProxy<uint16_t>::write(uint16_t v) {
    mr().halfword(addr) = v;
}
template <> void mem::MemoryImage::MemoryCellProxy<uint32_t>::write(uint32_t v) {
    mr().word(addr) = v;
}
template <> void mem::MemoryImage::MemoryCellProxy<uint64_t>::write(uint64_t v) {
    mr().doubleword(addr) = v;
}
