#ifndef MINICCOMPILER_SYMBOL_HPP
#define MINICCOMPILER_SYMBOL_HPP

#include "constraints.hpp"

struct Symbol {
    TokenType type;
    int stackOffset;

    Symbol() = default;
    Symbol(const TokenType type, const int offset)
        : type(type), stackOffset(offset)
    {}

    Symbol(const TokenType type, int *current_offset) : type(type) {
        switch (type) {
            case DOUBLE: {
                stackOffset = *current_offset - DOUBLE_SIZE_BYTES;
                *current_offset -= DOUBLE_SIZE_BYTES;
                break;
            }
            case INT: {
                stackOffset = *current_offset - INT_SIZE_BYTES;
                *current_offset -= INT_SIZE_BYTES;
                break;
            }
            case FLOAT: {
                stackOffset = *current_offset - FLOAT_SIZE_BYTES;
                *current_offset -= FLOAT_SIZE_BYTES;
                break;
            }
            default: {
                stackOffset = *current_offset - 1;
                *current_offset -= 1;
                break;
            }
        }
    }
};

#endif //MINICCOMPILER_SYMBOL_HPP