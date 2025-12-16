#ifndef __PARSER_H__
#define __PARSER_H__

#include "stmt.hpp"
#include "lexer.hpp"

#include <iostream>
#include <optional>

enum StatementType {
    VarDeclaration,
    AssignmentStatement,

};

class Parser {
private:
    std::vector<std::vector<Token>> code;

    std::vector<Token> actual_instruction;
    std::size_t pc;     // conta a instrução atual
    std::size_t pos;    // conta o token atual

    std::optional<Token> current();
    Token consume();
    bool match(TokenType type) const;

    Expr *parseExpression();
    Expr *parseTerm();
    Expr *parseFactor();

public:
    Parser(std::vector<std::vector<Token>> code)
        : code(code), actual_instruction(code[0]), pc(0), pos(0)
    {};

    Stmt *parseStatement();
    bool next_instruction() {
        if (++pc >= code.size()) return false;
        actual_instruction = code[pc];
        pos = 0;

        return true;
    };

    bool previous_instruction() {
        if (--pc < 0) return false;
        actual_instruction = code[pc];

        return true;
    }

    void reset_instructions() {
        pc = 0;
        pos = 0;
        actual_instruction = code[pc];
    }
};

#endif