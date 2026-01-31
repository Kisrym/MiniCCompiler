#ifndef MINICCOMPILER_SYMBOL_HPP
#define MINICCOMPILER_SYMBOL_HPP

#include "constraints.hpp"
#include "value.hpp"
#include "token.hpp"

struct FuncDefStmt;

/*enum ValueKind { // tipos de "registradores" que estao armazenados os dados
    TEMP_REG,
    ARG_REG,
    STACK,
    ZERO
};*/

struct Symbol {
    TokenType type = UNKNOWN;
    int stackOffset = 0;
    Storage location = Storage::REGISTER;

    Symbol() = default;
    Symbol(const TokenType type, const int offset)
        : type(type), stackOffset(offset)
    {}

    Symbol(const TokenType type, int *current_offset) : type(type) {
        switch (type) {
            case DOUBLE: {
                stackOffset = *current_offset + DOUBLE_SIZE_BYTES;
                *current_offset += DOUBLE_SIZE_BYTES;
                break;
            }
            case INT: {
                stackOffset = *current_offset + INT_SIZE_BYTES;
                *current_offset += INT_SIZE_BYTES;
                break;
            }
            case FLOAT: {
                stackOffset = *current_offset + FLOAT_SIZE_BYTES;
                *current_offset += FLOAT_SIZE_BYTES;
                break;
            }
            case STRING: {
                stackOffset = *current_offset + STRING_SIZE_BYTES;
                *current_offset += STRING_SIZE_BYTES;
                break;
            }
            default: {
                stackOffset = *current_offset + 1;
                *current_offset += 1;
                break;
            }
        }
    }
};

struct FunctionSymbol {
    TokenType type;
    FuncDefStmt *definition;

    FunctionSymbol() = default;
    FunctionSymbol(const TokenType type, FuncDefStmt *definition)
        : type(type), definition(definition)
    {}
};

#endif //MINICCOMPILER_SYMBOL_HPP