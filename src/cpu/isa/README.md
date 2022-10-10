# ISA

R_TYPE(name, opcode, funct7, funct3, execution code, precedence)
I_TYPE(name, opcode, funct3, execution code, precedence)
S_TYPE(name, opcode, funct3, execution code, precedence)
B_TYPE(name, opcode, funct3, execution code, precedence)
U_TYPE(name, opcode, execution code, precedence)
J_TYPE(name, opcode, execution code, precedence)
CUSTOM(name, opcode, matcher code, execution code, precedence)

FOR PRECEDENCE
lower value is of higher precedence.
This simplifies most matching, as a precedence of 0 is always taken

execution code has access to a HartState object with register file, memory, and PC


ALL REGISTER READS MUST OCCUR BEFORE ANY REGISTER WRITES


immediate shifts are weird, they are kinda I type, kinda R type


# registers

REGISTER_CLASS(classname, reg_prefix, number_regs, reg_size)
REGISTER(classname, number, nice_name)
