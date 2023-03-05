#ifndef ZIRCON_HART_ISA_INSTRUCTION_MATCH_H_
#define ZIRCON_HART_ISA_INSTRUCTION_MATCH_H_

#include <cstdint>

namespace instruction {
using Word = uint32_t;

// get a mask with N bits
template <std::size_t N> constexpr Word getMask() {
    static_assert(N <= sizeof(Word) * 8, "Invalid Bit Length");
    return 0xFFFFFFFFULL >> ((sizeof(Word) * 8) - N);
}

template <std::size_t LSB, std::size_t COUNT>
constexpr Word getBitsFromLSB(Word bits) {
    static_assert(LSB < sizeof(Word) * 8, "Invalid Index");
    return (bits >> LSB) & getMask<COUNT>();
}
template <std::size_t LSB, std::size_t COUNT>
constexpr Word clearBitsFromLSB(Word bits) {
    static_assert(LSB < sizeof(Word) * 8, "Invalid Index");
    return bits & ~(getMask<COUNT>() << LSB);
}
template <std::size_t LSB, std::size_t COUNT>
constexpr Word setBitsFromLSB(Word bits, Word toSet) {
    static_assert(LSB < sizeof(Word) * 8, "Invalid Index");
    Word toSetMask = toSet & getMask<COUNT>();
    Word bitsClear = clearBitsFromLSB<LSB, COUNT>(bits);
    Word bitsSet = bitsClear | (toSetMask << LSB);
    return bitsSet;
}

template <std::size_t N> constexpr Word getBit(Word bits) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    return getBitsFromLSB<N, 1>(bits);
}
template <std::size_t N> constexpr Word clearBit(Word bits) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    return clearBitsFromLSB<N, 1>(bits);
}
template <std::size_t N> constexpr Word setBit(Word bits, Word toSet) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    return setBitsFromLSB<N, 1>(bits, toSet);
}

template <std::size_t MSB, std::size_t COUNT>
constexpr Word getBitsFromMSB(Word bits) {
    static_assert(MSB < sizeof(Word) * 8, "Invalid Index");
    return getBitsFromLSB<MSB - COUNT + 1, COUNT>(bits);
}
template <std::size_t MSB, std::size_t COUNT>
constexpr Word clearBitsFromMSB(Word bits) {
    static_assert(MSB < sizeof(Word) * 8, "Invalid Index");
    return clearBitsFromLSB<MSB - COUNT + 1, COUNT>(bits);
}
template <std::size_t MSB, std::size_t COUNT>
constexpr Word setBitsFromMSB(Word bits, Word toSet) {
    static_assert(MSB < sizeof(Word) * 8, "Invalid Index");
    return setBitsFromLSB<MSB - COUNT + 1, COUNT>(bits, toSet);
}

// inclusive, MSB to LSB
template <std::size_t MSB, std::size_t LSB>
constexpr Word getBitRange(Word bits) {
    static_assert(MSB < sizeof(Word) * 8 && MSB >= LSB, "Invalid Indexes");
    return getBitsFromLSB<LSB, MSB - LSB + 1>(bits);
}
template <std::size_t MSB, std::size_t LSB>
constexpr Word clearBitRange(Word bits) {
    static_assert(MSB < sizeof(Word) * 8 && MSB >= LSB, "Invalid Indexes");
    return clearBitsFromLSB<LSB, MSB - LSB + 1>(bits);
}
template <std::size_t MSB, std::size_t LSB>
constexpr Word setBitRange(Word bits, Word toSet) {
    static_assert(MSB < sizeof(Word) * 8 && MSB >= LSB, "Invalid Indexes");
    return setBitsFromLSB<LSB, MSB - LSB + 1>(bits, toSet);
}

template <std::size_t B> constexpr __int128_t signext128(uint64_t value) {
    static_assert(B <= 128 && B >= 0, "Invalid Size");
    return __int128_t(value);
    // return __int128_t(value << (128 - B)) >> (128 - B);
}
template <std::size_t B> constexpr int64_t signext64(uint64_t value) {
    static_assert(B <= 64 && B >= 0, "Invalid Size");
    return int64_t(value << (64 - B)) >> (64 - B);
}
template <std::size_t B> constexpr int32_t signext32(uint32_t value) {
    static_assert(B <= 32 && B >= 0, "Invalid Size");
    return int32_t(value << (32 - B)) >> (32 - B);
}

constexpr Word getOpcode(Word bits) { return getBitsFromLSB<0, 7>(bits); }
constexpr Word setOpcode(Word bits, Word toSet) {
    return setBitsFromLSB<0, 7>(bits, toSet);
}

constexpr Word getFunct7(Word bits) { return getBitsFromLSB<25, 7>(bits); }
constexpr Word setFunct7(Word bits, Word toSet) {
    return setBitsFromLSB<25, 7>(bits, toSet);
}

constexpr Word getFunct3(Word bits) { return getBitsFromLSB<12, 3>(bits); }
constexpr Word setFunct3(Word bits, Word toSet) {
    return setBitsFromLSB<12, 3>(bits, toSet);
}

constexpr Word getRd(Word bits) { return getBitsFromLSB<7, 5>(bits); }
constexpr Word setRd(Word bits, Word toSet) {
    return setBitsFromLSB<7, 5>(bits, toSet);
}

constexpr Word getRs2(Word bits) { return getBitsFromLSB<20, 5>(bits); }
constexpr Word setRs2(Word bits, Word toSet) {
    return setBitsFromLSB<20, 5>(bits, toSet);
}

constexpr Word getRs1(Word bits) { return getBitsFromLSB<15, 5>(bits); }
constexpr Word setRs1(Word bits, Word toSet) {
    return setBitsFromLSB<15, 5>(bits, toSet);
}

constexpr Word getShamt5(Word bits) { return getBitsFromLSB<20, 5>(bits); }
constexpr Word setShamt5(Word bits, Word toSet) {
    return setBitsFromLSB<20, 5>(bits, toSet);
}

constexpr Word getShamt6(Word bits) { return getBitsFromLSB<20, 6>(bits); }
constexpr Word setShamt6(Word bits, Word toSet) {
    return setBitsFromLSB<20, 6>(bits, toSet);
}

constexpr Word getITypeImm(Word bits) { return getBitsFromLSB<20, 12>(bits); }
constexpr Word setITypeImm(Word bits, Word toSet) {
    return setBitsFromLSB<20, 12>(bits, toSet);
}

constexpr Word getSTypeImm(Word bits) {
    Word bits25_7 = getBitsFromLSB<25, 7>(bits);
    Word bits7_5 = getBitsFromLSB<7, 5>(bits);

    Word imm11_5 = setBitRange<11, 5>(0, bits25_7);
    Word imm4_0 = setBitRange<4, 0>(0, bits7_5);
    return imm11_5 | imm4_0;
}
constexpr Word setSTypeImm(Word bits, Word toSet) {
    Word imm11_5 = getBitRange<11, 5>(toSet);
    Word imm4_0 = getBitRange<4, 0>(toSet);

    bits = setBitsFromLSB<25, 7>(bits, imm11_5);
    bits = setBitsFromLSB<7, 5>(bits, imm4_0);
    return bits;
}

constexpr Word getBTypeImm(Word bits) {
    Word bits31 = getBit<31>(bits);
    Word bits30_25 = getBitRange<30, 25>(bits);
    Word bits11_8 = getBitRange<11, 8>(bits);
    Word bits7 = getBit<7>(bits);

    Word imm12 = setBit<12>(0, bits31);
    Word imm10_5 = setBitRange<10, 5>(0, bits30_25);
    Word imm4_1 = setBitRange<4, 1>(0, bits11_8);
    Word imm11 = setBit<11>(0, bits7);

    return imm12 | imm10_5 | imm4_1 | imm11;
}
constexpr Word setBTypeImm(Word bits, Word toSet) {
    Word imm12 = getBit<12>(toSet);
    Word imm10_5 = getBitRange<10, 5>(toSet);
    Word imm4_1 = getBitRange<4, 1>(toSet);
    Word imm11 = getBit<11>(toSet);

    bits = setBit<31>(bits, imm12);
    bits = setBitRange<30, 25>(bits, imm10_5);
    bits = setBitRange<11, 8>(bits, imm4_1);
    bits = setBit<7>(bits, imm11);
    return bits;
}
constexpr Word getUTypeImm(Word bits) {
    Word bits12_20 = getBitsFromLSB<12, 20>(bits);
    Word imm31_12 = setBitRange<31, 12>(0, bits12_20);
    return imm31_12;
}
constexpr Word setUTypeImm(Word bits, Word toSet) {
    Word imm31_12 = getBitRange<31, 12>(toSet);
    bits = setBitsFromLSB<12, 20>(bits, imm31_12);
    return bits;
}
constexpr Word getJTypeImm(Word bits) {
    Word bits31 = getBit<31>(bits);
    Word bits21_10 = getBitsFromLSB<21, 10>(bits);
    Word bits20 = getBit<20>(bits);
    Word bits12_8 = getBitsFromLSB<12, 8>(bits);

    Word imm20 = setBit<20>(0, bits31);
    Word imm10_1 = setBitRange<10, 1>(0, bits21_10);
    Word imm11 = setBit<11>(0, bits20);
    Word imm19_12 = setBitRange<19, 12>(0, bits12_8);

    return imm20 | imm10_1 | imm11 | imm19_12;
}
constexpr Word setJTypeImm(Word bits, Word toSet) {

    Word imm20 = getBit<20>(toSet);
    Word imm10_1 = getBitRange<10, 1>(toSet);
    Word imm11 = getBit<11>(toSet);
    Word imm19_12 = getBitRange<19, 12>(toSet);

    bits = setBit<31>(bits, imm20);
    bits = setBitsFromLSB<21, 10>(bits, imm10_1);
    bits = setBit<20>(bits, imm11);
    bits = setBitsFromLSB<12, 8>(bits, imm19_12);

    return bits;
}
}; // namespace instruction

#endif
