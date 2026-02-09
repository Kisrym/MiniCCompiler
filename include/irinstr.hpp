//
// Created by kaio on 08/02/2026.
//

#ifndef MINICCOMPILER_IRINSTR_HPP
#define MINICCOMPILER_IRINSTR_HPP

enum class Op {
    CONST,
    LOAD,
    STORE,

    ADD,
    SUB,
    MUL,
    DIV,

    AND,
    OR,
    XOR,

    LT,
    GT,
    LE,
    GE,
    EQ,
    NE,

    NEG,
    NOT,

    CALL,
    RET,

    LABEL,
    JUMP,
    JUMP_IF_ZERO,
    BRANCH
};

struct IRInstr {
    Op op;

    int rd = -1;
    int rs1 = -1;
    int rs2 = -1;

    int offset = -1;
    std::string label;
    std::string func;
    std::vector<int> args;
};

#endif //MINICCOMPILER_IRINSTR_HPP