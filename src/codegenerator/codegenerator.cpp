#include "codegenerator.hpp"

CodeGenerator::CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer)
    : parser(parser), analyzer(semanticAnalyzer), currentStackOffset(0), pos(0)
{
    t_register = labels = 0;

    do {
        Stmt *current_statement = parser->parseStatement();
        instructions.push_back(analyzer->analyze(current_statement)); // armazena as instruções com as anotações do semantic analyzer
    } while (parser->next_instruction());

    current_instruction = instructions[0];
    FRAME_SIZE = analyzer->get_current_offset() - 4;
}

std::vector<std::string> CodeGenerator::generateCode() {
    // settar o prologo do programa (configuração de sp e fp)
    const std::string fs_str = std::to_string(FRAME_SIZE);
    const std::string fs4_str = std::to_string(4);

    generated_code.push_back("addi sp, sp, " + fs_str + "\nsw fp, " + fs4_str + "(sp)\nmv fp, sp\n\n");

    for (Stmt *stmt : instructions) {
        genStmt(stmt);
        generated_code.push_back(result_bff);
        result_bff.clear();
    }

    generated_code.push_back("\nmv sp, fp\nlw fp, " + fs4_str + "(sp)\naddi sp, sp, " + fs_str.substr(1) + "\n");
    generated_code.emplace_back("li a7, 10\necall\n"); // exit program rars

    return generated_code;
}
// expressions
unsigned int CodeGenerator::genExpr(Expr *expression) {
    if (const auto expr = dynamic_cast<IntExpr *>(expression)) {
        return genLiteral(expr); // por enquanto é somente esse
    }

    if (const auto expr = dynamic_cast<VarExpr *>(expression)) {
        return genVarExpr(expr);
    }

    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr);
    }

    if (const auto expr = dynamic_cast<BoolExpr *>(expression)) {
        return genBoolExpr(expr);
    }

    if (const auto expr = dynamic_cast<UnaryExpr *>(expression)) {
        return genUnaryExpr(expr);
    }

    return -1;
}

unsigned int CodeGenerator::genExpr(Expr *expression, const std::string &label_else, const std::string &label_if) {
    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr, label_else, label_if);
    }

    return genExpr(expression);
}

unsigned int CodeGenerator::genLiteral(const IntExpr *expr) {
    const unsigned int tmp = t_register;

    result_bff += "li " + get_reg_name(t_register) + ", " + std::to_string(expr->value) + "\n";
    t_register++;

    return tmp;
}

unsigned int CodeGenerator::genBoolExpr(const BoolExpr *expr) {
    const unsigned int tmp = t_register;

    if (const unsigned int conversion = expr->value) {
        result_bff += "li " + get_reg_name(t_register) + ", " + std::to_string(conversion) + "\n";
        t_register++;

        return tmp;
    }

    return 100; // vale x0 (zero)
}

unsigned int CodeGenerator::genVarExpr(const VarExpr *expr) {
    const unsigned tmp = t_register;
    result_bff += "lw " + get_reg_name(t_register) + ", " + std::to_string(expr->symbol->stackOffset) + "(fp)\n";
    t_register++;

    return tmp;
}

unsigned int CodeGenerator::genBinaryExpr(const BinaryExpr *expr) {
    const unsigned left = genExpr(expr->value1);
    const unsigned right = genExpr(expr->value2);

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

        case AND:
            command = "and ";
            break;

        case OR:
            command = "or ";
            break;

        case EQUAL:
        case NEQUAL:
            command = "xor ";
            break;

        case GREATER:
        case LESSER:
        case GE:
        case LE:
            command = "slt ";
            break;

        default:
            command = "";
    }

    const std::string rd = get_reg_name(left);
    if (op == GREATER || op == LE) {
        result_bff += command + rd + ", " + get_reg_name(right) + ", " + rd + "\n"; // inverte a fim de simular comparações de maior que
        t_register--;

        if (op == LE) {
            result_bff += "xori " + rd + ", " + rd + ", 1\n";
        }
    }

    else {
        result_bff += command + rd + ", " + rd + ", " + get_reg_name(right) + "\n";
        t_register--; // libera o registrador

        if (op == EQUAL) {
            result_bff += "sltiu " + rd + ", " + rd + ", 1\n"; // (xor x, y == 0) ? x == y : x != y. basicamente, essa instrução verifica isso
        }
        else if (op == NEQUAL) {
            result_bff += "sltu " + rd + ", zero, " + rd + "\n";
        }
    }

    return left;
}

unsigned int CodeGenerator::genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if) {
    if (expr->op.type == AND || expr->op.type == OR) {
        op_stack.push(expr->op.type);
    }
    const unsigned int left = genExpr(expr->value1, label_else, label_if);
    const unsigned int right = genExpr(expr->value2, label_else, label_if);

    const TokenType op = expr->op.type;

    std::string command;

    // aqui vão estar ao contrário para mais fácil conversão à lógica do asm
    if (!op_stack.empty() && op_stack.top() == OR) {
        switch (op) {
            case LESSER:
                command = "blt ";
                break;

            case GREATER:
                command = "bgt ";
                break;

            case LE: // <=
                command = "ble ";
                break;

            case GE: // >=
                command = "bge ";
                break;

            case EQUAL:
                command = "beq ";
                break;

            case NEQUAL:
                command = "bne ";
                break;

            default:
                command = "";
                break;
        }
    }
    else {
        switch (op) {
            case LESSER:
                command = "bge ";
                break;

            case GREATER:
                command = "ble ";
                break;

            case LE: // <=
                command = "bgt ";
                break;

            case GE: // >=
                command = "blt ";
                break;

            case EQUAL:
                command = "bne ";
                break;

            case NEQUAL:
                command = "beq ";
                break;

            default:
                command = "";
                break;
        }
    }

    const std::string rd = get_reg_name(left);

    // a forma como serão feitos os saltos simularão o AND e OR
    // se ambos pularem pro ELSE, será um AND, se pularem pro IF, será um OR
    if (!op_stack.empty() && op_stack.top() == OR){
        result_bff += command + get_reg_name(left) + ", " + get_reg_name(right) + ", " + label_if + "\n";
    }

    else if (!command.empty()){
        result_bff += command + get_reg_name(left) + ", " + get_reg_name(right) + ", " + label_else + "\n";
    }

    if (!op_stack.empty())
        op_stack.pop();

    t_register--; // libera o registrador
    return left;
}

unsigned int CodeGenerator::genUnaryExpr(const UnaryExpr *expr) {
    const TokenType op = expr->op.type;

    const unsigned int rd = genExpr(expr->value1);
    const std::string rd_name = get_reg_name(rd);

    if (op == MINUS) {
        result_bff += "sub " + rd_name + "zero, " + rd_name + "\n";

        return rd;
    }

    // caso default: NOT
    result_bff += "xori " + rd_name + ", " + rd_name + ", -1\n";

    return rd;
}

// statements
std::string CodeGenerator::genStmt(Stmt *statement) {
    if (const auto stmt = dynamic_cast<VarDeclStmt *>(statement)) {
        return genVarDecl(stmt);
    }

    if (const auto stmt = dynamic_cast<AssignStmt *>(statement)) {
        return genAssign(stmt);
    }

    if (const auto stmt = dynamic_cast<IfStmt *>(statement)) {
        return genIfStmt(stmt);
    }

    if (const auto stmt = dynamic_cast<ForStmt *>(statement)) {
        return genForStmt(stmt);
    }

    return "";
}

std::string CodeGenerator::genVarDecl(const VarDeclStmt *statement) {
    // salva o resultado da expressao
    result_bff += "sw " + get_reg_name(genExpr(statement->expression)) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    t_register--;

    return result_bff;
}

std::string CodeGenerator::genAssign(const AssignStmt *statement) {
    const std::string rd = get_reg_name(t_register);

    // carrega a variavel pro assign
    result_bff += "lw " + get_reg_name(t_register) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    t_register++; // aloca um reg pra ela

    const std::string rs1 = get_reg_name(genExpr(statement->expression)); // armazena o reg com o resultado da expressão em rs1
    t_register--;

    result_bff += "add " + rd + ", zero, " + rs1 + "\n"; // OTIMIZAÇÃO DPS
    result_bff += "sw " + rd + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n"; // salva dnv
    t_register--;

    return result_bff;
}

std::string CodeGenerator::genIfStmt(const IfStmt *statement) {
    const std::string label_else = "ELSE" + std::to_string(labels);
    const std::string label_end = "ENDIF" + std::to_string(labels);
    const std::string label_start = "STARTIF" + std::to_string(labels);

    labels++;
    genExpr(statement->condition, label_else, label_start); // gera o if (na pratica so vai pular pro else)

    result_bff += label_start + ": \n";
    for (const auto &stmt : statement->thenBranches) {
        genStmt(stmt);
    }

    result_bff += "jal zero, " + label_end + "\n" + label_else + ": \n";
    for (const auto &stmt : statement->elseBranches) {
        genStmt(stmt);
    }
    result_bff += label_end + ": \n";

    return result_bff;
}

std::string CodeGenerator::genForStmt(const ForStmt *statement) {
    const std::string label_for = "FOR" + std::to_string(labels);
    const std::string label_end = "ENDFOR" + std::to_string(labels);

    labels++;
    result_bff += label_for + ": \n";
    genExpr(statement->condition, label_end, label_for); // gera a condicao
    for (const auto &stmt : statement->body) {
        genStmt(stmt);
    }
    genStmt(statement->increment);

    result_bff += "jal zero, " + label_for + "\n" + label_end + ": \n";
    return result_bff;
}