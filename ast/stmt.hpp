#ifndef __STMT_H__
#define __STMT_H__

#include <utility>

#include "lexer.hpp"
#include "expr.hpp"
#include "astnode.hpp"

class Stmt : public ASTNode {
public:
    Token type;
    virtual ~Stmt() = default;
};

// nó para declaração de variáveis

// int x = 5;
struct VarDeclStmt : public Stmt {
    Token type;
    std::string id;
    Expr *expression;

    Symbol *symbol;

    explicit VarDeclStmt(Token type, std::string id, Expr *expression)
        : type(std::move(type)), id(std::move(id)), expression(expression)
    {};

    ~VarDeclStmt() override {
        delete expression;
        delete symbol;
    }
};

struct AssignStmt : public Stmt {
    std::string id;
    Expr *expression;

    Symbol *symbol;

    explicit AssignStmt(std::string id, Expr *expression)
        : id(std::move(id)), expression(expression)
    {};

    ~AssignStmt() override {
        delete expression;
        delete symbol;
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

struct FuncDefStmt : public Stmt {
    Token type;
    std::string id;
    std::vector<VarDeclStmt *> parameters;
    std::vector<Stmt *> body;
    int FRAMESIZE = 0;

    explicit FuncDefStmt(Token type, std::string id, std::vector<VarDeclStmt *> parameters, std::vector<Stmt *> body)
        : type(std::move(type)), id(std::move(id)), parameters(std::move(parameters)), body(std::move(body))
    {}
};

struct RetStmt : public Stmt {
    Expr *expression;
    TokenType inferredType;

    explicit RetStmt(Expr *expression)
        : expression(expression)
    {};
};

struct FuncCallStmt : public Stmt {
    FuncDefStmt *definition;
    std::vector<Expr *> arguments;

    FuncCallStmt(FuncDefStmt *definition, std::vector<Expr *> parameters)
        : definition(definition), arguments(std::move(parameters))
    {}
};

#endif