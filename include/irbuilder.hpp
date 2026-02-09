#ifndef MINICCOMPILER_IRBUILDER_HPP
#define MINICCOMPILER_IRBUILDER_HPP

#include <vector>

#include "expr.hpp"
#include "irinstr.hpp"
#include "parser.hpp"
#include "semantic_analyzer.hpp"
#include "stmt.hpp"

class IRBuilder {
    Parser *parser;
    SemanticAnalyzer *analyzer;

    std::vector<IRInstr> ir;
    FuncDefStmt *actual_func = nullptr;
    int t_register = -1;
    int label = 0;

    int newRegister() {return ++t_register;}
    std::string newLabel(const std::string &name) const {return name + std::to_string(label);}

    void genStmt(Stmt *stmt);
    int genExpr(Expr *expr);

public:
    IRBuilder(Parser *parser, SemanticAnalyzer *analyzer);

    std::vector<IRInstr> build() {return ir;}
    static void printIR(const std::vector<IRInstr>& ir) { // TEMPORARIO
        for (auto& i : ir) {
            switch (i.op) {
                case Op::CONST:
                    std::cout << "t" << i.rd << " = const " << i.rs1 << "\n";
                    break;

                case Op::ADD:
                    std::cout << "t" << i.rd << " = add t"
                              << i.rs1 << " t" << i.rs2 << "\n";
                    break;

                case Op::LOAD:
                    std::cout << "t" << i.rd << " = load [fp"
                              << i.offset << "]\n";
                    break;

                case Op::STORE:
                    std::cout << "store t" << i.rs1
                              << " -> [fp" << i.offset << "]\n";
                    break;

                case Op::LABEL:
                    std::cout << i.label << ":\n";
                    break;

                case Op::JUMP:
                    std::cout << "jump " << i.label << "\n";
                    break;

                case Op::JUMP_IF_ZERO:
                    std::cout << "jz t" << i.rs1
                              << " -> " << i.label << "\n";
                    break;

                case Op::RET:
                    std::cout << "ret t" << i.rs1 << "\n";
                    break;

                case Op::CALL:
                    std::cout << "t" << i.rd << " = call " << i.func << "(";
                    for (const auto &inn : i.args) {
                        std::cout << "t" << inn << ", ";
                    }
                    std::cout << ")\n";
                    break;

                case Op::SUB:
                case Op::MUL:
                case Op::DIV:
                case Op::AND:
                case Op::OR:

                case Op::EQ:
                case Op::NE:
                case Op::GT:
                case Op::LT:
                case Op::GE:
                case Op::LE:
                    std::cout << "t" << i.rd << " = BINARYEXPR t"
                              << i.rs1 << " t" << i.rs2 << "\n";
                    break;
            }
        }
    }

};

#endif //MINICCOMPILER_IRBUILDER_HPP