#ifndef ZIRCON_CPU_ISA_INSTRUCTION_MATCH_H_
#define ZIRCON_CPU_ISA_INSTRUCTION_MATCH_H_

#include <cstdint>

namespace instruction {
using Word = uint32_t;

// get a mask with N bits
template <std::size_t N> constexpr Word getMask() {
    static_assert(N <= sizeof(Word) * 8, "Invalid Bit Length");
    return (1UL << N) - 1;
}
template <std::size_t N, std::size_t COUNT>
constexpr Word getBitsFromLSB(Word bits) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    return (bits >> N) & getMask<COUNT>();
}
template <std::size_t N> constexpr Word getBit(Word bits) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    // return (bits >> N) & 1;
    return getBitsFromLSB<N, 1>(bits);
}
template <std::size_t N, std::size_t COUNT>
constexpr Word getBitsFromMSB(Word bits) {
    static_assert(N < sizeof(Word) * 8, "Invalid Index");
    return getBitsFromLSB<N - COUNT + 1, COUNT>(bits);
}
// inclusive
template <std::size_t N, std::size_t M> constexpr Word getBitRange(Word bits) {
    static_assert(N < sizeof(Word) * 8 && N >= M, "Invalid Indexes");
    return (bits >> M) & getMask<N - M + 1>();
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
constexpr Word getFunct7(Word bits) { return getBitsFromLSB<25, 7>(bits); }
constexpr Word getFunct3(Word bits) { return getBitsFromLSB<12, 3>(bits); }
constexpr Word getRd(Word bits) { return getBitsFromLSB<7, 5>(bits); }
constexpr Word getRs2(Word bits) { return getBitsFromLSB<20, 5>(bits); }
constexpr Word getRs1(Word bits) { return getBitsFromLSB<15, 5>(bits); }
constexpr Word getShamt5(Word bits) { return getBitsFromLSB<20, 5>(bits); }
constexpr Word getShamt6(Word bits) { return getBitsFromLSB<20, 6>(bits); }

constexpr Word getITypeImm(Word bits) { return getBitsFromLSB<20, 12>(bits); }
constexpr Word getSTypeImm(Word bits) {
    return (/*imm11_5*/ getBitsFromLSB<25, 7>(bits) << 5) |
           (/*imm4_0*/ getBitsFromLSB<7, 5>(bits));
}
constexpr Word getBTypeImm(Word bits) {
    return (/*imm12*/ getBitsFromLSB<31, 1>(bits) << 12) |
           (/*imm10_5*/ getBitsFromLSB<25, 6>(bits) << 5) |
           (/*imm4_1*/ getBitsFromLSB<8, 4>(bits) << 1) |
           (/*imm11*/ getBitsFromLSB<7, 1>(bits) << 11);
}
constexpr Word getUTypeImm(Word bits) {
    return (/*imm31_12*/ getBitsFromLSB<12, 20>(bits) << 12);
}
constexpr Word getJTypeImm(Word bits) {
    return (/*imm20*/ getBitsFromLSB<31, 1>(bits) << 20) |
           (/*imm10_1*/ getBitsFromLSB<21, 10>(bits) << 1) |
           (/*imm11*/ getBitsFromLSB<20, 1>(bits) << 11) |
           (/*imm19_12*/ getBitsFromLSB<12, 8>(bits) << 12);
}
}; // namespace instruction

#endif
