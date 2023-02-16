#include "expr.h"

#include "hart/hartstate.h"

namespace command {
types::SignedInteger Expr::eval(hart::HartState* hs) {
    if(hs == nullptr) return 0;
    return 0; // TODO: implement meco
}
} // namespace command
