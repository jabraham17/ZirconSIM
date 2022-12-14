
#ifndef REGISTER_CLASS
    #define REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)
#endif

// #ifndef REGISTER
//     #define REGISTER(classname, number, nice_name, initial_value, rd_only)
// #endif

#define MAKE_REGISTER(number, reg_size, nice_name, initial_value, rd_only)     \
    Register<reg_size>(initial_value, #nice_name, rd_only)

#define REGISTER_CLASS_GPR(R)                                                  \
    R(0, 64, zero, 0, true)                                                    \
    R(1, 64, ra, 0, false)                                                     \
    R(2, 64, sp, 0, false)                                                     \
    R(3, 64, gp, 0, false)                                                     \
    R(4, 64, tp, 0, false)                                                     \
    R(5, 64, t0, 0, false)                                                     \
    R(6, 64, t1, 0, false)                                                     \
    R(7, 64, t2, 0, false)                                                     \
    R(8, 64, s0, 0, false)                                                     \
    R(9, 64, s1, 0, false)                                                     \
    R(10, 64, a0, 0, false)                                                    \
    R(11, 64, a1, 0, false)                                                    \
    R(12, 64, a2, 0, false)                                                    \
    R(13, 64, a3, 0, false)                                                    \
    R(14, 64, a4, 0, false)                                                    \
    R(15, 64, a5, 0, false)                                                    \
    R(16, 64, a6, 0, false)                                                    \
    R(17, 64, a7, 0, false)                                                    \
    R(18, 64, s2, 0, false)                                                    \
    R(19, 64, s3, 0, false)                                                    \
    R(20, 64, s4, 0, false)                                                    \
    R(21, 64, s5, 0, false)                                                    \
    R(22, 64, s6, 0, false)                                                    \
    R(23, 64, s7, 0, false)                                                    \
    R(24, 64, s8, 0, false)                                                    \
    R(25, 64, s9, 0, false)                                                    \
    R(26, 64, s10, 0, false)                                                   \
    R(27, 64, s11, 0, false)                                                   \
    R(28, 64, t3, 0, false)                                                    \
    R(29, 64, t4, 0, false)                                                    \
    R(30, 64, t5, 0, false)                                                    \
    R(31, 64, t6, 0, false)
REGISTER_CLASS(GPR, x, 32, 64)

#undef REGISTER_CLASS
// #undef REGISTER
