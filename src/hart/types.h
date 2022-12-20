#ifndef ZIRCON_HART_TYPES_
#define ZIRCON_HART_TYPES_

#include <cstdint>
#include <string>

namespace types {
using Address = uint64_t;
using RegisterIndex = uint64_t;
using Integer = uint64_t;
using SignedInteger = int64_t;

Address strToAddress(const std::string& s) ;
RegisterIndex strToRegisterIndex(const std::string& s);
Integer strToInteger(const std::string& s) ;
SignedInteger strToSignedInteger(const std::string& s);

}

#endif
