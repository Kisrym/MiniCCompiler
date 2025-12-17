#ifndef MINICCOMPILER_CODEGENERATOR_HPP
#define MINICCOMPILER_CODEGENERATOR_HPP

#include <stack>

#include "parser.hpp"
#include "semantic_analyzer.hpp"
#include "stmt.hpp"

class CodeGenerator {
    Parser *parser;
    SemanticAnalyzer *analyzer;

    std::vector<Stmt *> instructions;
    Stmt *current_instruction;
    std::vector<std::string> generated_code;

    int FRAME_SIZE;

    int currentStackOffset;
    int pos;
    std::string result_bff;

    unsigned int t_register;
    unsigned int labels;
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

    // expressions
    unsigned int genExpr(Expr *expression);
    unsigned int genExpr(Expr *expression, const std::string &label_else, const std::string &label_if);

    unsigned int genLiteral(const IntExpr *expr);
    unsigned int genVarExpr(const VarExpr *expr);
    unsigned int genBoolExpr(const BoolExpr *expr);
    unsigned int genBinaryExpr(const BinaryExpr *expr);
    unsigned int genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if);
    unsigned int genUnaryExpr(const UnaryExpr *expr);

    // util
    static std::string get_reg_name(const unsigned int reg) {
        if (reg == 100) return "zero";
        if (reg > 6) return "OVERFLOW";
        return {'t', static_cast<char>(reg + '0')};
    }
public:
    CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer);

    std::vector<std::string> generateCode();

};

#endif //MINICCOMPILER_CODEGENERATOR_HPP