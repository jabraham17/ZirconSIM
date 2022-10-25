#ifndef ZIRCON_CONTROLLER_CONTROLLER_H_
#define ZIRCON_CONTROLLER_CONTROLLER_H_

#include "cpu/cpu.h"
#include <functional>
#include <ostream>
#include <vector>

namespace controller {

using GeneralCallback = void (*)(cpu::HartState&);
using LoggingCallback = void (*)(cpu::HartState&, std::ostream&);


} // namespace controller

#endif
