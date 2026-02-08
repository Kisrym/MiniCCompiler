#include "irbuilder.hpp"

#include <stdexcept>

int IRBuilder::genExpr(Expr *expr) {
    if (const auto expression = dynamic_cast<IntExpr *>(expr)) {
        const int rd = new_register();

        ir.push_back(IRInstr{Op::CONST, rd, expression->value});
        return rd;
    }

    if (const auto expression = dynamic_cast<BoolExpr *>(expr)) {
        const int rd = new_register();

        ir.push_back(IRInstr{Op::CONST, rd, expression->value});
        return rd;
    }

    if (const auto expression = dynamic_cast<VarExpr *>(expr)) {
        const int rd = new_register();

        ir.push_back(IRInstr{.op = Op::LOAD, .rd = rd, .offset = expression->symbol->stackOffset});
        return rd;
    }

    if (const auto expression = dynamic_cast<BinaryExpr*>(expr)) {
        const int lhs = genExpr(expression->value1);
        const int rhs = genExpr(expression->value2);

        const int rd = new_register();

        switch (expression->op.type) {
            case PLUS: ir.push_back({Op::ADD, rd, lhs, rhs}); break;
            case MINUS: ir.push_back({Op::SUB, rd, lhs, rhs}); break;
            case MUL: ir.push_back({Op::MUL, rd, lhs, rhs}); break;
            case DIV: ir.push_back({Op::DIV, rd, lhs, rhs}); break;
            case AND: ir.push_back({Op::AND, rd, lhs, rhs}); break;
            case OR: ir.push_back({Op::OR, rd, lhs, rhs}); break;

            case EQUAL: ir.push_back({Op::EQ, rd, lhs, rhs}); break;
            case NEQUAL: ir.push_back({Op::NE, rd, lhs, rhs}); break;
            case GREATER: ir.push_back({Op::GT, rd, lhs, rhs}); break;
            case LESSER: ir.push_back({Op::LT, rd, lhs, rhs}); break;
            case GE: ir.push_back({Op::GE, rd, lhs, rhs}); break;
            case LE: ir.push_back({Op::LE, rd, lhs, rhs}); break;
            default: return rd;
        }

        return rd;
    }

    if (const auto expression = dynamic_cast<UnaryExpr *>(expr)) {
        const int rd = genExpr(expression->value1);

        if (expression->op.type == MINUS) {
            ir.push_back(IRInstr{Op::NEG, rd});
            return rd;
        }

        ir.push_back(IRInstr{Op::NOT, rd});
        return rd;
    }

    if (const auto expression = dynamic_cast<FuncCallExpr *>(expr)) {
        std::vector<int> args;
        for (const auto &arg : expression->arguments) {
            args.push_back(genExpr(arg));
        }

        const int rd = new_register();
        ir.push_back(IRInstr{.op = Op::CALL, .rd = rd, .func = expression->definition->id, .args = args});
        return rd;
    }

    throw std::runtime_error("IRBuilder: Invalid expression");
}

void IRBuilder::genStmt(Stmt *stmt) {
    if (const auto statement = dynamic_cast<VarDeclStmt *>(stmt)) {
        const int rd = new_register();

        ir.push_back(IRInstr{Op::STORE, rd, genExpr(statement->expression)});
    }

    if (const auto statement = dynamic_cast<AssignStmt *>(stmt)) {
        const int rd = new_register();

        ir.push_back(IRInstr{Op::STORE, rd, genExpr(statement->expression)});
    }

    if (const auto statement = dynamic_cast<IfStmt *>(stmt)) {

    }
}
