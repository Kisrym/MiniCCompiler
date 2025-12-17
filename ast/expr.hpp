#ifndef __EXPR_H__
#define __EXPR_H__

#include "astnode.hpp"
#include "lexer.hpp"
#include "symbol.hpp"
#include <string>
#include <utility>

class Expr : public ASTNode {
public:
    virtual ~Expr() = default;
};

struct NumExpr : public Expr {
    virtual ~NumExpr() = default;
};

struct IntExpr : public NumExpr {
    int value;
    IntExpr(const int value) : value(value) {};
};

struct DoubleExpr : public NumExpr {
    double value;

    DoubleExpr(const double value) : value(value) {};
};

struct BoolExpr : public Expr {
    bool value;

    BoolExpr(const bool value) : value(value) {};
};

struct StringExpr : public Expr {
    std::string value;

    StringExpr(std::string value) : value(std::move(value)) {};
};

struct VarExpr : public Expr {
    std::string value;
    TokenType inferredType;
    Symbol *symbol;

    VarExpr(std::string value) : value(std::move(value)) {};
};

// "==" "+" "-", ...
struct BinaryExpr : public Expr {
    Expr *value1;
    Expr *value2;
    Token op;
    TokenType inferredType; // tipo inferido que vai vir da análise semântica a fim de auxiliar na geração de codigo

    //friend std::ostream& operator<<(std::ostream &os, const BinaryExpr &expr);

    BinaryExpr(Expr *value1, Expr *value2, Token op)
        : value1(value1), value2(value2), op(op)
    {};

    ~BinaryExpr() override {
        delete value1;
        delete value2;
    };

};

struct UnaryExpr : public Expr {
    Expr *value1;
    Token op;
    TokenType inferredType;

    UnaryExpr(Expr *value1, Token op)
        : value1(value1), op(std::move(op)) {}

    ~UnaryExpr() override {
        delete value1;
    }
};

#endif