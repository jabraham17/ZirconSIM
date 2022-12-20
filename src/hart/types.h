#ifndef ZIRCON_HART_TYPES_
#define ZIRCON_HART_TYPES_

#include <cstdint>
#include <string>

namespace types {
using Address = uint64_t;
using RegisterIndex = uint64_t;
using UnsignedInteger = uint64_t;
using SignedInteger = int64_t;
using InstructionWord = uint32_t;

Address strToAddress(const std::string& s);
RegisterIndex strToRegisterIndex(const std::string& s);
UnsignedInteger strToUnsignedInteger(const std::string& s);
SignedInteger strToSignedInteger(const std::string& s);

} // namespace types

#endif
