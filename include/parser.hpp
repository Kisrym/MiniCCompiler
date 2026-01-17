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
    std::vector<Token> code;
    std::size_t pos;    // conta o token atual

    std::vector<FunctionSymbol> functions;

    std::optional<Token> current();
    Token consume();
    bool match(TokenType type) const;

    Expr *parseExpression();
    Expr *parseTerm();
    Expr *parseFactor();

    Stmt *parseFunction(const Token &type, const Token &id);
    Expr *parseFunction(const Token &id);

public:
    explicit Parser(const std::vector<Token> &code)
        : code(code), pos(0) {
    };

    Stmt *parseStatement();
};

#endif