#ifndef MINICCOMPILER_VALUE_HPP
#define MINICCOMPILER_VALUE_HPP

enum class Storage {
    REGISTER,
    STACK,
    IMMEDIATE,
};

struct value {
    Storage storage; // onde o valor está armazenado
    int reg; // index do registrador, se for o caso
    int offset; // offset da stack, se for o caso
    int imm; // valor do imediato, se for o caso
};

#endif //MINICCOMPILER_VALUE_HPP