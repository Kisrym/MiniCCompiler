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

    inline friend std::ostream& operator<<(std::ostream &os, const IntExpr &expr) {
        os << expr.value;
        return os;
    };

    IntExpr(int value) : value(value) {};
};

struct DoubleExpr : public NumExpr {
    double value;

    inline friend std::ostream& operator<<(std::ostream &os, const DoubleExpr &expr) {
        os << expr.value;
        return os;
    };

    DoubleExpr(const double value) : value(value) {};
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

inline std::ostream& operator<<(std::ostream &os, const BinaryExpr &expr) {
    if (!expr.value1 || !expr.value2) {
        return os << "ERRO: ponteiro nulo em BinaryExpr";
    }

    if (auto i = dynamic_cast<IntExpr*>(expr.value1))
        os << *i;
    else if (auto d = dynamic_cast<DoubleExpr*>(expr.value1))
        os << *d;
    else if (auto s = dynamic_cast<StringExpr*>(expr.value1))
        os << "\"" << s->value << "\"";
    else if (auto b = dynamic_cast<BinaryExpr*>(expr.value1))
        os << *b;
    else
        os << "<Expr desconhecida>";

    os << " " << expr.op << " ";

    if (auto i = dynamic_cast<IntExpr*>(expr.value2))
        os << *i;
    else if (auto d = dynamic_cast<DoubleExpr*>(expr.value2))
        os << *d;
    else if (auto s = dynamic_cast<StringExpr*>(expr.value2))
        os << "\"" << s->value << "\"";
    else if (auto b = dynamic_cast<BinaryExpr*>(expr.value2))
        os << *b;
    else
        os << "<Expr desconhecida>";

    return os;
}
#endif