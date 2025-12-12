#ifndef __STMT_H__
#define __STMT_H__

#include <utility>

#include "lexer.hpp"
#include "expr.hpp"
#include "astnode.hpp"

class Stmt : public ASTNode {
public:
    virtual ~Stmt() = default;
};

// nó para declaração de variáveis

// int x = 5;
struct VarDeclStmt : public Stmt {
    Token type;
    std::string id;
    Expr *expression;

    VarDeclStmt(Token type, std::string id, Expr *expression)
        : type(std::move(type)), id(std::move(id)), expression(expression)
    {};

    ~VarDeclStmt() override {
        delete expression;
    }
};

struct AssignStmt : public Stmt {
    std::string id;
    Expr *expression;

    AssignStmt(std::string id, Expr *expression)
        : id(std::move(id)), expression(expression)
    {};

    ~AssignStmt() override {
        delete expression;
    }
};

struct IfStmt : public Stmt {
    Expr *condition;
    std::vector<Stmt *> thenBranches;
    std::vector<Stmt *> elseBranches;

    IfStmt(Expr *condition, std::vector<Stmt *> thenBranches, std::vector<Stmt *> elseBranches)
        : condition(condition), thenBranches(std::move(thenBranches)), elseBranches(std::move(elseBranches))
    {}
};

struct ForStmt : public Stmt {
    Stmt *definition;
    Expr *condition;
    AssignStmt *increment;
    std::vector<Stmt *> body;

    ForStmt(Stmt *definition, Expr *condition, AssignStmt *increment, std::vector<Stmt *> body)
        : definition(definition), condition(condition), increment(increment), body(std::move(body))
    {}
};

struct WhileStmt : public Stmt {
    Expr *condition;
    std::vector<Stmt *> body;

    WhileStmt(Expr *condition, std::vector<Stmt *> body)
        : condition(condition), body(std::move(body))
    {};
};

#endif