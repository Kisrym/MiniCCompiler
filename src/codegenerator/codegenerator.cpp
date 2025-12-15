#include "codegenerator.hpp"

CodeGenerator::CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer)
    : parser(parser), analyzer(semanticAnalyzer), currentStackOffset(0), pos(0), t_register(0)
{
    do {
        Stmt *current_statement = parser->parseStatement();
        instructions.push_back(analyzer->analyze(current_statement)); // armazena as instruções com as anotações do semantic analyzer
    } while (parser->next_instruction());

    current_instruction = instructions[0];
    FRAME_SIZE = analyzer->get_current_offset();
}

std::vector<std::string> CodeGenerator::generateCode() {
    // settar o prologo do programa (configuração de sp e fp)
    const std::string fs_str = std::to_string(FRAME_SIZE);
    const std::string fs4_str = std::to_string((-FRAME_SIZE) - 4);

    generated_code.push_back("addi sp, sp, " + fs_str + "\nsw fp, " + fs4_str + "(sp)\nmv fp, sp\n");

    for (Stmt *stmt : instructions) {
        genStmt(stmt);
        generated_code.push_back(result_bff);
        result_bff.clear();
    }

    generated_code.push_back("mv sp, fp\nlw fp, " + fs4_str + "(sp)\naddi sp, sp, " + fs4_str + "\n");
    generated_code.emplace_back("li a7, 10\necall\n"); // exit program rars

    return generated_code;
}
// expressions
int CodeGenerator::genExpr(Expr *expression) {
    if (const auto expr = dynamic_cast<IntExpr *>(expression)) {
        return genLiteral(expr); // por enquanto é somente esse
    }

    if (const auto expr = dynamic_cast<VarExpr *>(expression)) {
        return genVarExpr(expr);
    }

    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr);
    }

    return -1;
}

int CodeGenerator::genLiteral(const IntExpr *expr) {
    const int tmp = t_register;

    result_bff += "li " + get_reg_name(t_register) + ", " + std::to_string(expr->value) + "\n";
    alloc_reg();

    return tmp;
}

int CodeGenerator::genVarExpr(const VarExpr *expr) {
    const int tmp = t_register;
    result_bff += "lw " + get_reg_name(t_register) + ", " + std::to_string(expr->symbol->stackOffset) + "(fp)\n";
    alloc_reg();

    return tmp;
}

int CodeGenerator::genBinaryExpr(const BinaryExpr *expr) {
    const int left = genExpr(expr->value1);
    const int right = genExpr(expr->value2);

    const TokenType op = expr->op.type;

    std::string command;

    switch (op) {
        case PLUS:
            command = "add ";
            break;

        case MINUS:
            command = "sub ";
            break;

        case MUL:
            command = "mul ";
            break;

        case DIV:
            command = "div ";
            break;

        default:
            command = "";
    }

    const std::string rd = get_reg_name(left);
    result_bff += command + rd + ", " + rd + ", " + get_reg_name(right) + "\n";
    free_reg();

    return left;
}

// statements
std::string CodeGenerator::genStmt(Stmt *statement) {
    if (const auto stmt = dynamic_cast<VarDeclStmt *>(statement)) {
        return genVarDecl(stmt);
    }

    if (const auto stmt = dynamic_cast<AssignStmt *>(statement)) {
        //return genAssign(stmt);
        return genAssign(stmt);
    }

    return "";
}

std::string CodeGenerator::genVarDecl(const VarDeclStmt *statement) {
    // salva o resultado da expressao
    result_bff += "sw " + get_reg_name(genExpr(statement->expression)) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    free_reg();

    return result_bff;
}

std::string CodeGenerator::genAssign(const AssignStmt *statement) {
    const std::string rd = get_reg_name(t_register);

    // carrega a variavel pro assign
    result_bff += "lw " + get_reg_name(t_register) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    alloc_reg(); // aloca um reg pra ela

    const std::string rs1 = get_reg_name(genExpr(statement->expression)); // armazena o reg com o resultado da expressão em rs1
    free_reg();

    result_bff += "mv " + rd + ", " + rs1 + "\n"; // OTIMIZAÇÃO DPS
    result_bff += "sw " + rd + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n"; // salva dnv
    free_reg();

    return result_bff;
}