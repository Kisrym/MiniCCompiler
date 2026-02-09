#include "irbuilder.hpp"

#include <stdexcept>

IRBuilder::IRBuilder(Parser *parser, SemanticAnalyzer *analyzer)
    : parser(parser), analyzer(analyzer)
{
    while (Stmt *current_stmt = parser->parseStatement()) {
        genStmt(analyzer->analyze(current_stmt));
    }
}

int IRBuilder::genExpr(Expr *expr) {
    if (const auto expression = dynamic_cast<IntExpr *>(expr)) {
        const int rd = newRegister();

        ir.push_back(IRInstr{Op::CONST, rd, expression->value});
        return rd;
    }

    if (const auto expression = dynamic_cast<BoolExpr *>(expr)) {
        const int rd = newRegister();

        ir.push_back(IRInstr{Op::CONST, rd, expression->value});
        return rd;
    }

    if (const auto expression = dynamic_cast<VarExpr *>(expr)) {
        const int rd = newRegister();

        ir.push_back(IRInstr{.op = Op::LOAD, .rd = rd, .offset = expression->symbol->stackOffset});
        return rd;
    }

    if (const auto expression = dynamic_cast<BinaryExpr*>(expr)) {
        const int lhs = genExpr(expression->value1);
        const int rhs = genExpr(expression->value2);

        const int rd = newRegister();

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

        const int rd = newRegister();
        ir.push_back(IRInstr{.op = Op::CALL, .rd = rd, .func = expression->definition->id, .args = args});
        return rd;
    }

    throw std::runtime_error("IRBuilder: Invalid expression");
}

void IRBuilder::genStmt(Stmt *stmt) {
    if (const auto statement = dynamic_cast<VarDeclStmt *>(stmt)) {
        ir.push_back(IRInstr{.op = Op::STORE, .rs1 = genExpr(statement->expression), .offset = statement->symbol->stackOffset});
        return;
    }

    if (const auto statement = dynamic_cast<AssignStmt *>(stmt)) {
        const int rd = newRegister();

        ir.push_back(IRInstr{.op = Op::STORE, .rd = rd, .rs1 = genExpr(statement->expression), .offset = statement->symbol->stackOffset});
        return;
    }

    if (const auto statement = dynamic_cast<IfStmt *>(stmt)) {
        const std::string endIf = newLabel("ENDIF");
        const std::string elseIf = newLabel("ELSEIF");
        label++;

        const int cond = genExpr(statement->condition);
        ir.push_back(IRInstr{.op = Op::JUMP_IF_ZERO, .rs1 = cond, .label = elseIf});

        for (const auto i : statement->thenBranches) {
            genStmt(i);
        }

        ir.push_back(IRInstr{.op = Op::JUMP, .label = endIf});
        ir.push_back(IRInstr{.op = Op::LABEL, .label = elseIf});

        for (const auto i : statement->elseBranches) {
            genStmt(i);
        }

        ir.push_back(IRInstr{.op = Op::LABEL, .label = endIf});
        return;
    }

    if (const auto statement = dynamic_cast<ForStmt *>(stmt)) {
        const std::string startFor = newLabel("FOR");
        const std::string endFor = newLabel("ENDFOR");
        label++;

        genStmt(statement->definition);

        ir.push_back(IRInstr{.op = Op::LABEL, .label = startFor});

        const int cond = genExpr(statement->condition);
        ir.push_back(IRInstr{.op = Op::JUMP_IF_ZERO, .rs1 = cond, .label = endFor});

        for (const auto &st : statement->body) {
            genStmt(st);
        }

        genStmt(statement->increment);

        ir.push_back(IRInstr{.op = Op::JUMP, .label = startFor});
        ir.push_back(IRInstr{.op = Op::LABEL, .label = endFor});
        return;
    }

    if (const auto statement = dynamic_cast<WhileStmt *>(stmt)) {
        const std::string startWhile = newLabel("WHILE");
        const std::string endWhile = newLabel("ENDWHILE");
        label++;

        ir.push_back(IRInstr{.op = Op::LABEL, .label = startWhile});

        const int cond = genExpr(statement->condition);
        ir.push_back(IRInstr{.op = Op::JUMP_IF_ZERO, .rs1 = cond, .label = endWhile});

        for (const auto &st : statement->body) {
            genStmt(st);
        }

        ir.push_back(IRInstr{.op = Op::JUMP, .label = startWhile});
        ir.push_back(IRInstr{.op = Op::LABEL, .label = endWhile});
        return;
    }

    if (const auto statement = dynamic_cast<FuncDefStmt *>(stmt)) {
        t_register = -1; // reseta os temporarios

        ir.push_back(IRInstr{.op = Op::LABEL, .label = statement->id});

        for (const auto &st : statement->body) {
            genStmt(st);
        }

        t_register = -1; // reseta os temporarios (DEPOIS MODIFICAR -- INICIO DE CADA FUNÇÃO O AREA DE SCRIPT TEM QUE RESETAR TEMPS)
        return;
    }

    if (const auto statement = dynamic_cast<RetStmt *>(stmt)) {
        const int t = genExpr(statement->expression);
        ir.push_back(IRInstr{.op = Op::RET, .rs1 = t});
        return;
    }

    if (const auto statement = dynamic_cast<FuncCallStmt *>(stmt)) {
        std::vector<int> args;
        for (const auto &i : statement->arguments) {
            args.push_back(genExpr(i));
        }

        ir.push_back(IRInstr{.op = Op::CALL, .func = statement->definition->id, .args = args});
        return;
    }
}
