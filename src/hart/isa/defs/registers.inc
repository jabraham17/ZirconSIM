
#ifndef REGISTER_CLASS
    #define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)
#endif

// #ifndef REGISTER
//     #define REGISTER(classname, number, nice_name, initial_value, rd_only)
// #endif

#define MAKE_REGISTER(classname, number, nice_name, initial_value, rd_only)    \
    Register(                                                                  \
        rf::internal::registerSizeFor##classname,                              \
        initial_value,                                                         \
        #nice_name,                                                            \
        rd_only)

/*out of tree registers take precedence over ones defined here*/
/*this allows an application to override existing ones*/
#include "generated-definition/registers.inc"

#define REGISTER_CLASS_GPR(R)                                                  \
    R(GPR, 0, zero, 0, true)                                                   \
    R(GPR, 1, ra, 0, false)                                                    \
    R(GPR, 2, sp, 0, false)                                                    \
    R(GPR, 3, gp, 0, false)                                                    \
    R(GPR, 4, tp, 0, false)                                                    \
    R(GPR, 5, t0, 0, false)                                                    \
    R(GPR, 6, t1, 0, false)                                                    \
    R(GPR, 7, t2, 0, false)                                                    \
    R(GPR, 8, s0, 0, false)                                                    \
    R(GPR, 9, s1, 0, false)                                                    \
    R(GPR, 10, a0, 0, false)                                                   \
    R(GPR, 11, a1, 0, false)                                                   \
    R(GPR, 12, a2, 0, false)                                                   \
    R(GPR, 13, a3, 0, false)                                                   \
    R(GPR, 14, a4, 0, false)                                                   \
    R(GPR, 15, a5, 0, false)                                                   \
    R(GPR, 16, a6, 0, false)                                                   \
    R(GPR, 17, a7, 0, false)                                                   \
    R(GPR, 18, s2, 0, false)                                                   \
    R(GPR, 19, s3, 0, false)                                                   \
    R(GPR, 20, s4, 0, false)                                                   \
    R(GPR, 21, s5, 0, false)                                                   \
    R(GPR, 22, s6, 0, false)                                                   \
    R(GPR, 23, s7, 0, false)                                                   \
    R(GPR, 24, s8, 0, false)                                                   \
    R(GPR, 25, s9, 0, false)                                                   \
    R(GPR, 26, s10, 0, false)                                                  \
    R(GPR, 27, s11, 0, false)                                                  \
    R(GPR, 28, t3, 0, false)                                                   \
    R(GPR, 29, t4, 0, false)                                                   \
    R(GPR, 30, t5, 0, false)                                                   \
    R(GPR, 31, t6, 0, false)
REGISTER_CLASS(GPR, x, 32, 64)

#undef REGISTER_CLASS
// #undef REGISTER
