#ifndef MINICCOMPILER_CODEGENERATOR_HPP
#define MINICCOMPILER_CODEGENERATOR_HPP

#include <stack>

#include "parser.hpp"
#include "semantic_analyzer.hpp"
#include "stmt.hpp"

struct Value {
    ValueKind kind;
    int index; // index do registrador ou offset da stack
};

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

    int t_register = 0;
    int arg_regs = 0;
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
    Value genExpr(Expr *expression);
    Value genExpr(Expr *expression, const std::string &label_else, const std::string &label_if);

    Value genLiteral(const IntExpr *expr);
    Value genVarExpr(const VarExpr *expr);
    Value genBoolExpr(const BoolExpr *expr);
    Value genBinaryExpr(const BinaryExpr *expr);
    Value genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if);
    Value genUnaryExpr(const UnaryExpr *expr);
    Value genStringExpr(const StringExpr *expr);
    Value genFuncCallExpr(const FuncCallExpr *expr);

    // util
    static std::string get_reg_name(const Value &value) {
        if (value.kind == ZERO) return "zero";

        if (value.kind == TEMP_REG || value.kind == STACK) {
            if (value.index > 6) return "OVERFLOW";
            return {'t', static_cast<char>(value.index + '0')};
        }

        if (value.kind == ARG_REG) {
            if (value.index > 7) return "OVERFLOW";
            return {'a', static_cast<char>(value.index + '0')};
        }

        return "INVALID KIND";
    }

    std::string load(const Value &v, bool is_arg_reg = false);

public:
    CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer);

    std::vector<std::string> generateCode();

};

#endif //MINICCOMPILER_CODEGENERATOR_HPP