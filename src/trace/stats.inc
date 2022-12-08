
#ifndef COUNTER
    #define COUNTER(name, expression)
#endif
#ifndef COMPUTED
    #define COMPUTED(name, expression)
#endif

COUNTER("all instructions executed", counter++;)
COUNTER("R-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isRType()) counter++;)
COUNTER("I-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isIType()) counter++;)
COUNTER("S-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isSType()) counter++;)
COUNTER("B-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isBType()) counter++;)
COUNTER("U-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isUType()) counter++;)
COUNTER("J-Type instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op.isJType()) counter++;)
COUNTER("ebreak/ecall instructions executed", auto bits = hs().getInstWord();
        auto op = isa::inst::decodeInstruction(bits);
        if(op == isa::inst::Opcode::rv32i_ebreak ||
           op == isa::inst::Opcode::rv32i_ecall) counter++;)

COMPUTED("percent R-Type",
         return (counters["R-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent I-Type",
         return (counters["I-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent S-Type",
         return (counters["S-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent B-Type",
         return (counters["B-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent U-Type",
         return (counters["U-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent J-Type",
         return (counters["J-Type instructions executed"] /
                 counters["all instructions executed"]) *
                100;)
COMPUTED("percent ebreak/ecall",
         return (counters["ebreak/ecall instructions executed"] /
                 counters["all instructions executed"]) *
                100;)

#undef COUNTER
#undef COMPUTED
