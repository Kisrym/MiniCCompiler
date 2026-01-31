#ifndef MINICCOMPILER_CODEGENERATOR_HPP
#define MINICCOMPILER_CODEGENERATOR_HPP

#include <stack>

#include "parser.hpp"
#include "semantic_analyzer.hpp"
#include "stmt.hpp"
#include "value.hpp"
#include "../src/codegenerator/registerpool.hpp"

/*
 * LEMBRETE: adicioar uma nova função a fim de evitar manipulação direta da string "text" ou "data". Funcionaria como
 *
 * emit("mv a0, t0")
 *
 * ou algo assim
 */

class CodeGenerator {
    Parser *parser;
    SemanticAnalyzer *analyzer;

    std::vector<Stmt *> instructions;
    Stmt *current_instruction;
    std::vector<std::string> generated_code;

    int FRAME_SIZE = 0;

    int currentStackOffset = 0;
    int pos = 0;
    std::string text_bff;
    std::string data_bff;
    std::string functions_bff; // vao ficar no final do .text

    RegisterPool temps_pool;
    RegisterPool args_pool;
    unsigned int l_string = 0;
    unsigned int labels = 0;
    std::stack<TokenType> op_stack;

    bool next_instruction() {
        if (++pos >= instructions.size()) return false;
        current_instruction = instructions[pos];
        return true;
    }

    // statements
    std::string genStmt(Stmt *statement);
    std::string genVarDecl(const VarDeclStmt *statement);
    std::string genAssign(const AssignStmt *statement);
    std::string genIfStmt(const IfStmt *statement);
    std::string genForStmt(const ForStmt *statement);
    std::string genWhileStmt(const WhileStmt *statement);
    std::string genFuncDefStmt(const FuncDefStmt *statement);
    std::string genFuncCallStmt(const FuncCallStmt *statement);
    std::string genRetStmt(const RetStmt *statement);

    // expressions
    value genExpr(Expr *expression);
    value genExpr(Expr *expression, const std::string &label_else, const std::string &label_if);

    value genLiteral(const IntExpr *expr);
    value genVarExpr(const VarExpr *expr);
    value genBoolExpr(const BoolExpr *expr);
    value genBinaryExpr(const BinaryExpr *expr);
    value genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if);
    value genUnaryExpr(const UnaryExpr *expr);
    value genStringExpr(const StringExpr *expr);
    value genFuncCallExpr(const FuncCallExpr *expr);


    std::string load(const value &v);

public:
    CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer);

    std::vector<std::string> generateCode();

};

#endif //MINICCOMPILER_CODEGENERATOR_HPP