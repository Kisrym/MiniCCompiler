#ifndef MINICCOMPILER_CODEGENERATOR_HPP
#define MINICCOMPILER_CODEGENERATOR_HPP

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
    int t_register;
    std::string result_bff;

    bool next_instruction() {
        if (++pos >= instructions.size()) return false;
        current_instruction = instructions[pos];
        return true;
    }

    // statements
    std::string genStmt(Stmt *statement);
    std::string genVarDecl(const VarDeclStmt *statement);
    std::string genAssign(const AssignStmt *statement);


    // expressions
    int genExpr(Expr *expression);
    int genLiteral(const IntExpr *expr);
    int genVarExpr(const VarExpr *expr);
    int genBinaryExpr(const BinaryExpr *expr);

    // util
    void alloc_reg() {
        t_register++;
    }

    void free_reg() {
        t_register--;
    }

    static std::string get_reg_name(const int reg) {
        switch (reg) {
            case 0: return "t0";
            case 1: return "t1";
            case 2: return "t2";
            case 3: return "t3";
            case 4: return "t4";
            case 5: return "t5";
            case 6: return "t6";
            default: return "OVERFLOW";
        }
    }
public:
    CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer);

    std::vector<std::string> generateCode();

};

#endif //MINICCOMPILER_CODEGENERATOR_HPP