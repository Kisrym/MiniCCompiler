#ifndef EXPR_H_
#define EXPR_H_

#include "astnode.hpp"
#include "symbol.hpp"
#include <string>
#include <utility>

class Expr : public ASTNode {
public:
    TokenType inferredType = UNKNOWN; // tipo inferido que vai vir da análise semântica a fim de auxiliar na geração de codigo
    ~Expr() override = default;
};

struct NumExpr : public Expr {
    ~NumExpr() override = default;
};

struct IntExpr : public NumExpr {
    int value;
    explicit IntExpr(const int value) : value(value) {};
};

struct DoubleExpr : public NumExpr {
    double value;

    explicit DoubleExpr(const double value) : value(value) {};
};

struct BoolExpr : public Expr {
    bool value;

    explicit BoolExpr(const bool value) : value(value) {};
};

struct StringExpr : public Expr {
    std::string value;

    explicit StringExpr(std::string value) : value(std::move(value)) {};
};

struct VarExpr : public Expr {
    std::string value;
    Symbol *symbol = nullptr;

    explicit VarExpr(std::string value) : value(std::move(value)) {};
};

// "==" "+" "-", ...
struct BinaryExpr : public Expr {
    Expr *value1;
    Expr *value2;
    Token op;

    //friend std::ostream& operator<<(std::ostream &os, const BinaryExpr &expr);

    explicit BinaryExpr(Expr *value1, Expr *value2, Token op)
        : value1(value1), value2(value2), op(std::move(op))
    {};

    ~BinaryExpr() override {
        delete value1;
        delete value2;
    };

};

struct UnaryExpr : public Expr {
    Expr *value1;
    Token op;

    explicit UnaryExpr(Expr *value1, Token op)
        : value1(value1), op(std::move(op)) {}

    ~UnaryExpr() override {
        delete value1;
    }
};

struct FuncCallExpr : public Expr {
    FuncDefStmt *definition;
    std::vector<Expr *> arguments;

    FuncCallExpr(FuncDefStmt *definition, std::vector<Expr *> parameters)
        : definition(definition), arguments(std::move(parameters))
    {}
};

#endif