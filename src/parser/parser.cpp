#include "parser.hpp"

std::optional<Token> Parser::current() {
    if (pos < code.size())
        return code[pos];

    return std::nullopt;
}

Token Parser::consume() {
    return code[pos++];
}

bool Parser::match(const TokenType type) const {
    return (pos < code.size() && code[pos].type == type);
}

Expr *Parser::parseFactor() {
    if (match(INT)) {
        return new IntExpr(std::stoi(consume().value)); // consome e já vai pro proximo
    }

    if (match(DOUBLE)) {
        return new DoubleExpr(std::stod(consume().value));
    }

    if (match(STRING)) {
        return new StringExpr(consume().value);
    }

    if (match(BOOL)) {
        return new BoolExpr(consume().value == "true");
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

    if (match(MINUS) || match(NOT)) {
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

    while (match(PLUS) || match(MINUS) || match(OR) || match(AND)) {
        Token op = consume();
        Expr *right = parseTerm();
        left = new BinaryExpr(left, right, op);
    }

    return left;
}

Stmt *Parser::parseStatement() {
    // se acabou os statements
    if (!current().has_value()) {
        return nullptr;
    }

    const Token first = consume(); // pode ser ou o tipo ou o nome

    if (first.type == RETURN) {
        Expr *expression = parseExpression();
        consume(); // ;
        return new RetStmt(expression);
    }

    if (first.type == INT || first.type == DOUBLE || first.type == STRING || first.type == BOOL || first.type == VOID) {
        const Token id = consume();

        if (current().has_value() && current().value().type == PAREN1) {
            return parseFunction(first, id);
        }

        // será inicializado com 0 padrão
        if (current().has_value() && current().value().type != ASSIGN) {
            if (first.type == STRING) return new VarDeclStmt(first, id.value, new StringExpr(""));
            if (first.type == DOUBLE) return new VarDeclStmt(first, id.value, new DoubleExpr(0.0));
            if (first.type == BOOL) return new VarDeclStmt(first, id.value, new BoolExpr(false));

            consume(); // ; ou )

            return new VarDeclStmt(first, id.value, new IntExpr(0));
        }
        consume(); // =
        Expr *expression = parseExpression();
        consume(); // ;

        return new VarDeclStmt(first, id.value, expression);
    }

    if (first.type == IF) {
        consume(); // (
        Expr *condition = parseExpression();
        consume(); // )
        consume(); // {

        std::vector<Stmt *> thenBranches;
        std::vector<Stmt *> elseBranches;

        while (current().has_value() && current()->type != BRACES2) {
            thenBranches.push_back(parseStatement());
        }

        consume(); // }

        if (current().has_value() && current()->type == ELSE) {
            consume(); // ELSE
            consume(); // {

            while (current().has_value() && current()->type != BRACES2) {
                elseBranches.push_back(parseStatement());
            }

            consume(); // }
        }

        return new IfStmt(condition, thenBranches, elseBranches);
    }

    if (first.type == WHILE) {
        consume(); // (
        Expr *condition = parseExpression();
        consume(); // )
        consume(); // {

        std::vector<Stmt *> whileBranches;
        while (current().has_value() && current()->type != BRACES2) {
            whileBranches.push_back(parseStatement());
            consume(); // ;
        }

        return new WhileStmt(condition, whileBranches);
    }

    if (first.type == FOR) {
        consume(); // (
        Stmt *definition = parseStatement();
        consume(); // ,
        Expr *condition = parseExpression();
        consume(); // ,
        auto *increment =  dynamic_cast<AssignStmt *>(parseStatement());
        consume(); // )
        consume(); // {

        std::vector<Stmt *> statements;
        while (current().has_value() && current()->type != BRACES2) {
            statements.push_back(parseStatement());
            consume(); // ;
        }

        return new ForStmt(definition, condition, increment, statements);
    }

    if (first.type == ELSE) {
        throw std::runtime_error("Illegal instruction: no 'if' linked.");
    }

    consume(); // =
    Expr *expression = parseExpression();
    consume(); // ;

    return new AssignStmt(first.value, expression);
}

Stmt *Parser::parseFunction(const Token &type, const Token &id) {
    std::vector<Stmt *> parameters;
    std::vector<Stmt *> body;

    consume(); // (
    while (current().has_value() && current()->type != BRACES1) {
        parameters.push_back(parseStatement());
    }

    consume(); // {
    while (current().has_value() && current()->type != BRACES2) {
        body.push_back(parseStatement());
    }
    consume(); // }

    return new FuncStmt(type, id.value, parameters, body);
}