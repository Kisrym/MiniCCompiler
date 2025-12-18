#include "parser.hpp"

std::optional<Token> Parser::current() {
    if (pos < actual_instruction.size())
        return actual_instruction[pos];

    return std::nullopt;
}

Token Parser::consume() {
    return actual_instruction[pos++];
}

bool Parser::match(const TokenType type) const {
    return (pos < actual_instruction.size() && actual_instruction[pos].type == type);
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
    // VAR DECL
    const Token first = consume(); // pode ser ou o tipo ou o nome

    if (first.type == INT || first.type == DOUBLE || first.type == STRING || first.type == BOOL) {
        const Token id = consume();

        // será inicializado com 0 padrão
        if (current().has_value() && current().value().type != ASSIGN) {
            return new VarDeclStmt(first, id.value, new IntExpr(0));
        }
        consume(); // =

        return new VarDeclStmt(first, id.value, parseExpression());
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
            consume(); // ;
        }

        consume(); // }

        if (current().has_value() && current()->type == ELSE) {
            consume(); // ELSE
            consume(); // {

            while (current().has_value() && current()->type != BRACES2) {
                elseBranches.push_back(parseStatement());
                consume(); // ;
            }

            consume(); // }
        }

        else { // CORRIGIR ESSA PARTE DEPOIS: MODIFICAR MELHOR O LEXER
            next_instruction(); // agora é tratado o else

            if (current().has_value() && current()->type == ELSE) {
                consume();
                consume();

                while (current().has_value() && current()->type != BRACES2) {
                    elseBranches.push_back(parseStatement());
                    consume(); // ;
                }
                consume(); // }
            }

            else previous_instruction(); // se a instrução atual não for um else, o if não tem um else linkado
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
            if (current().has_value() && current()->type == SEMI) {
                consume(); // em alguns casos, o próprio parseStatement vai tratar code blocks, nesse caso,
            }

        }

        return new ForStmt(definition, condition, increment, statements);
    }

    if (first.type == ELSE) {
        throw std::runtime_error("Illegal instruction: no 'if' linked.");
    }

    consume();

    return new AssignStmt(first.value, parseExpression());
}
