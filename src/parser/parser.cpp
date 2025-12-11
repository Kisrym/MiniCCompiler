#include "parser.hpp"

Token &Parser::current() {
    return actual_instruction[pos];
}

Token Parser::consume() {
    return actual_instruction[pos++];
}

bool Parser::match(const TokenType type) const {
    return (pos < actual_instruction.size() && actual_instruction[pos].type == type);
}

Expr *Parser::parseFactor() {
    if (match(NUM)) {
        return new DoubleExpr(std::stoi(consume().value)); // consome e já vai pro proximo
    }

    if (match(VALUE)) {
        return new StringExpr(consume().value);
    }

    if (match(PAREN1)) {
        consume(); // (
        Expr *expression = parseExpression();
        consume(); // )

        return expression;
    }

    if (match(BRACES1)) {
        consume();
        Expr *expression = parseExpression();

        return expression;
    }

    if (match(ID)) {
        return new VarExpr(consume().value);
    }

    if (match(MINUS)) {
        const Token op = consume();
        return new UnaryExpr(parseFactor(), op);
    }

    throw std::runtime_error("Invalid factor");
}

Expr *Parser::parseTerm(){
    Expr *left = parseFactor();

    while (match(MUL) || match(DIV) || match(LESSER) || match(LE) || match(GREATER)
        || match(GE) || match(EQUAL) || match(NEQUAL)) {
        Token op = consume();
        Expr *right = parseFactor();
        left = new BinaryExpr(left, right, op);
    }

    return left;
}

Expr *Parser::parseExpression() {
    Expr *left = parseTerm();

    while (match(PLUS) || match(MINUS)) {
        Token op = consume();
        Expr *right = parseTerm();
        left = new BinaryExpr(left, right, op);
    }

    return left;
}

Stmt *Parser::parseStatement() {
    // VAR DECL
    Token first = consume(); // pode ser ou o tipo ou o nome
    std::cout << first << std::endl;
    if (first.type == INT || first.type == DOUBLE || first.type == STRING || first.type == BOOL) {
        Token id = consume();
        consume(); // =

        return new VarDeclStmt(first, id.value, parseExpression());
    }

    if (first.type == IF) {
        consume(); // (
        Expr *condition = parseExpression();
        consume(); // )
        consume(); // {

        std::vector<Stmt *> statements;
        while (pos <= actual_instruction.size() && current().type != BRACES2) {
            statements.push_back(parseStatement());
            consume(); // ;
        }

        return new IfStmt(condition, statements);
    }

    if (first.type == ELSE) {
        consume(); // {

        std::vector<Stmt *> statements;
        while (pos < actual_instruction.size() && current().type != BRACES2) {
            statements.push_back(parseStatement());
            consume(); // ;
        }

        return new ElseStmt(statements);
    }

    consume();

    return new AssignStmt(first.value, parseExpression());
}
