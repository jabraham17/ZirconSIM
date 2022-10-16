#include "isa-begin.inc"

CUSTOM(rv32i,
       lr_w,
       0b0101111,
       return instruction::getOpcode(bits) == 0b0101111 &&
              instruction::getFunct3(bits) == 0b010 &&
              instruction::getBitsFromMSB<31 COMMA 5>(bits) == 0b00010;
       , std::stringstream ss;
    //    ss << "lr.w x" << instruction::getRd(bits) << ", x"               
    //        << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits);
       return ss.str();
       , std::cout << "unimp\n";
       hs.pc += 4;
       , 0)



       CUSTOM(rv32i,
       amoswap_w,
       0b0101111,
       return instruction::getOpcode(bits) == 0b0101111 &&
              instruction::getFunct3(bits) == 0b010 &&
              instruction::getBitsFromMSB<31 COMMA 5>(bits) == 0b00001;
       , std::stringstream ss;
       ss << "amoswap.w x" << instruction::getRd(bits) << ", x"               
           << instruction::getRs1(bits) << ", x" << instruction::getRs2(bits);
       return ss.str();
       , std::cout << "unimp\n";
       hs.pc += 4;
       , 0)
