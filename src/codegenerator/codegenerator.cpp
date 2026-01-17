#include "codegenerator.hpp"

CodeGenerator::CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer)
    : parser(parser), analyzer(semanticAnalyzer)
{

    while (Stmt *current_statement = parser->parseStatement()){
        instructions.push_back(analyzer->analyze(current_statement)); // armazena as instruções com as anotações do semantic analyzer
    }

    current_instruction = instructions[0];
    FRAME_SIZE = analyzer->get_current_offset() - 4;
}

std::vector<std::string> CodeGenerator::generateCode() {
    // settar o prologo do programa (configuração de sp e fp)
    const std::string fs_str = std::to_string(FRAME_SIZE);
    const std::string fs4_str = std::to_string(4);

    data_bff.append(".data\n");

    generated_code.emplace_back(".text\n");
    generated_code.push_back("\taddi sp, sp, " + fs_str + "\n\tsw fp, " + fs4_str + "(sp)\n\tadd fp, zero, sp\n\n");

    for (Stmt *stmt : instructions) {
        genStmt(stmt);
        generated_code.push_back(text_bff);
        text_bff.clear();
    }

    generated_code.push_back("\n\tadd sp, zero, fp\n\tlw fp, " + fs4_str + "(sp)\n\taddi sp, sp, " + fs_str.substr(1) + "\n");
    generated_code.emplace_back("\tli a7, 10\n\tecall\n"); // exit program rars

    generated_code.push_back(functions_bff);

    generated_code.insert(generated_code.cbegin(), data_bff);

    return generated_code;
}

std::string CodeGenerator::load(const Value &v, bool is_arg_reg) {
    if (v.kind == STACK) {
        auto valor = (is_arg_reg) ? Value{ARG_REG, arg_regs} : Value{TEMP_REG, t_register};
        return "lw " + get_reg_name(valor) + ", " + std::to_string(v.index) + "(fp)";
    }

    if (v.kind == ARG_REG) {
        //return "mv " + get_reg_name(Value(TEMP_REG, t_register)) + ", a" + std::to_string(v.index);
        return "mv " + get_reg_name(Value(ARG_REG, arg_regs)) + ", a" + std::to_string(v.index);
    }

    // se for temporario
    return "mv " + get_reg_name(Value(TEMP_REG, t_register)) + ", t" + std::to_string(v.index);
}

// expressions
Value CodeGenerator::genExpr(Expr *expression) {
    if (const auto expr = dynamic_cast<IntExpr *>(expression)) {
        return genLiteral(expr); // por enquanto é somente esse
    }

    if (const auto expr = dynamic_cast<StringExpr *>(expression)) {
        return genStringExpr(expr);
    }

    if (const auto expr = dynamic_cast<VarExpr *>(expression)) {
        return genVarExpr(expr);
    }

    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr);
    }

    if (const auto expr = dynamic_cast<UnaryExpr *>(expression)) {
        return genUnaryExpr(expr);
    }

    if (const auto expr = dynamic_cast<FuncCallExpr *>(expression)) {
        return genFuncCallExpr(expr);
    }

    return Value{TEMP_REG, -1};
}

Value CodeGenerator::genExpr(Expr *expression, const std::string &label_else, const std::string &label_if) {
    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr, label_else, label_if);
    }

    return genExpr(expression);
}

Value CodeGenerator::genLiteral(const IntExpr *expr) {
    const Value v(TEMP_REG, t_register);

    text_bff += "\tli " + get_reg_name(v) + ", " + std::to_string(expr->value) + "\n";
    t_register++;

    return v;
}

Value CodeGenerator::genBoolExpr(const BoolExpr *expr) {
    const Value v(TEMP_REG, t_register);

    if (const unsigned int conversion = expr->value) {
        text_bff += "\tli " + get_reg_name(v) + ", " + std::to_string(conversion) + "\n";
        t_register++;

        return v;
    }

    return Value{ZERO, 100}; // vale x0 (zero)
}

Value CodeGenerator::genStringExpr(const StringExpr *expr) {
    const Value v(TEMP_REG, t_register);
    const std::string string_label("STRING" + std::to_string(l_string));

    data_bff.append("\t"+ string_label + ": .asciz " + "\"" + expr->value +"\"\n");
    text_bff.append("\tla " + get_reg_name(v) + ", " + string_label + "\n");

    t_register++;
    l_string++;

    return v;
}

Value CodeGenerator::genVarExpr(const VarExpr *expr) {
    int *tmp = nullptr;

    if (expr->symbol->location == ARG_REG) {
        return Value{ARG_REG, arg_regs++};
    }

    const auto v = Value{expr->symbol->location, t_register++};

    text_bff += "\tlw " + get_reg_name(v) + ", " + std::to_string(expr->symbol->stackOffset) + "(fp)\n";
    //t_register++;

    return v;
}

Value CodeGenerator::genBinaryExpr(const BinaryExpr *expr) {
    const Value left = genExpr(expr->value1);
    const Value right = genExpr(expr->value2);

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
        text_bff += "\t" + command + rd + ", " + get_reg_name(right) + ", " + rd + "\n"; // inverte a fim de simular comparações de maior que
        t_register--;

        if (op == LE) {
            text_bff += "\txori " + rd + ", " + rd + ", 1\n";
        }
    }

    else {
        text_bff += "\t" + command + rd + ", " + rd + ", " + get_reg_name(right) + "\n";
        if (right.kind == ARG_REG) arg_regs--; // libera o registrador
        else t_register--;

        if (op == EQUAL) {
            text_bff += "\tsltiu " + rd + ", " + rd + ", 1\n"; // (xor x, y == 0) ? x == y : x != y. basicamente, essa instrução verifica isso
        }
        else if (op == NEQUAL) {
            text_bff += "\tsltu " + rd + ", zero, " + rd + "\n";
        }
    }

    return left;
}

Value CodeGenerator::genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if) {
    if (expr->op.type == AND || expr->op.type == OR) {
        op_stack.push(expr->op.type);
    }
    const Value left = genExpr(expr->value1, label_else, label_if);
    const Value right = genExpr(expr->value2, label_else, label_if);

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
        text_bff += "\t" + command + get_reg_name(left) + ", " + get_reg_name(right) + ", " + label_if + "\n";
    }

    else if (!command.empty()){
        text_bff += "\t" + command + get_reg_name(left) + ", " + get_reg_name(right) + ", " + label_else + "\n";
    }

    if (!op_stack.empty())
        op_stack.pop();

    if (right.kind == ARG_REG) arg_regs--; // libera o registrador
    else t_register--;
    return left;
}

Value CodeGenerator::genUnaryExpr(const UnaryExpr *expr) {
    const TokenType op = expr->op.type;

    const Value rd = genExpr(expr->value1);
    const std::string rd_name = get_reg_name(rd);

    if (op == MINUS) {
        text_bff += "\tsub " + rd_name + "zero, " + rd_name + "\n";

        return rd;
    }

    // caso default: NOT
    text_bff += "\txori " + rd_name + ", " + rd_name + ", -1\n";

    return rd;
}

Value CodeGenerator::genFuncCallExpr(const FuncCallExpr *expr) {
    for (auto &arg : expr->arguments) {
        if (const auto *expression = dynamic_cast<VarExpr *>(arg)) {
            expression->symbol->location = TEMP_REG;
        }

        text_bff += "\tmv " + get_reg_name(Value{ARG_REG, arg_regs++}) + ", " + get_reg_name(genExpr(arg)) + "\n";
        t_register--;
    }

    text_bff += "\tcall " + expr->definition->id + "\n";

    return Value{ARG_REG, 0}; // MODIFICAR TALVEZ PARA RETORNO DE 2 VARIAVEIS (a0, a1) => POSSIVELMENTE NAO
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

    if (const auto stmt = dynamic_cast<WhileStmt *>(statement)) {
        return genWhileStmt(stmt);
    }

    if (const auto stmt = dynamic_cast<FuncDefStmt *>(statement)) {
        return genFuncDefStmt(stmt);
    }

    if (const auto stmt = dynamic_cast<RetStmt *>(statement)) {
        return genRetStmt(stmt);
    }

    if (const auto stmt = dynamic_cast<FuncCallStmt *>(statement)) {
        return genFuncCallStmt(stmt);
    }

    return "";
}

std::string CodeGenerator::genVarDecl(const VarDeclStmt *statement) {
    // salva o resultado da expressao
    //text_bff += "\t" + load(genExpr(statement->expression)) + "\n";
    const auto value = genExpr(statement->expression);

    //text_bff += "\t" + load(Value{statement->symbol->location, statement->symbol->stackOffset}, value.kind == ARG_REG) + "\n";
    text_bff += "\tsw " + get_reg_name(value) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    if (value.kind == ARG_REG) arg_regs--;
    else t_register--;

    return text_bff;
}

std::string CodeGenerator::genAssign(const AssignStmt *statement) {
    const std::string rd = get_reg_name(Value(TEMP_REG, t_register));

    // carrega a variavel pro assign
    text_bff += "\t" + load(Value{STACK, statement->symbol->stackOffset}) + "\n";
    //text_bff += "\tlw " + get_reg_name(t_register) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";
    t_register++; // aloca um reg pra ela

    const std::string rs1 = get_reg_name(genExpr(statement->expression)); // armazena o reg com o resultado da expressão em rs1
    t_register--;

    text_bff += "\tadd " + rd + ", zero, " + rs1 + "\n"; // OTIMIZAÇÃO DPS
    text_bff += "\tsw " + rd + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n"; // salva dnv
    t_register--;

    return text_bff;
}

std::string CodeGenerator::genIfStmt(const IfStmt *statement) {
    const std::string label_else = "ELSE" + std::to_string(labels);
    const std::string label_end = "ENDIF" + std::to_string(labels);
    const std::string label_start = "STARTIF" + std::to_string(labels);

    labels++;
    genExpr(statement->condition, label_else, label_start); // gera o if (na pratica so vai pular pro else)

    text_bff += label_start + ": \n";
    for (const auto &stmt : statement->thenBranches) {
        genStmt(stmt);
    }

    text_bff += "\tjal zero, " + label_end + "\n" + label_else + ": \n";
    for (const auto &stmt : statement->elseBranches) {
        genStmt(stmt);
    }
    text_bff += label_end + ": \n";

    return text_bff;
}

std::string CodeGenerator::genForStmt(const ForStmt *statement) {
    const std::string label_for = "FOR" + std::to_string(labels);
    const std::string label_end = "ENDFOR" + std::to_string(labels);
    const std::string label_start = "STARTFOR" + std::to_string(labels);

    labels++;

    genStmt(statement->definition);
    text_bff += label_for + ": \n";
    genExpr(statement->condition, label_end, label_start); // gera a condicao

    for (const auto &stmt : statement->body) {
        genStmt(stmt);
    }
    genStmt(statement->increment);

    text_bff += "\tjal zero, " + label_for + "\n" + label_end + ": \n";
    return text_bff;
}

std::string CodeGenerator::genWhileStmt(const WhileStmt *statement) {
    const std::string label_while = "WHILE" + std::to_string(labels);
    const std::string label_end = "ENDWHILE" + std::to_string(labels);
    const std::string label_start = "STARTWHILE" + std::to_string(labels);

    labels++;
    text_bff += label_while + ": \n";
    genExpr(statement->condition, label_end, label_start); // gera o if (na pratica so vai pular pro else)

    text_bff += label_start + ": \n";
    for (const auto &stmt : statement->body) {
        genStmt(stmt);
    }

    text_bff += "\tjal zero, " + label_while + "\n" + label_end + ": \n";
    return text_bff;
}

std::string CodeGenerator::genFuncDefStmt(const FuncDefStmt *statement) {
    //functions_bff += "FUNCTION" + std::to_string(labels++) + ":\n";
    functions_bff += statement->id + ":\n";

    int FRAMESIZE = -statement->FRAMESIZE;
    FRAMESIZE = 16 * (((FRAMESIZE + 4) + 15) / 16); // -4 para armazenar ra e modulo para manter alinhamento de memoria
    FRAMESIZE = -FRAMESIZE;

    functions_bff += "\taddi sp, sp, " + std::to_string(FRAMESIZE) + "\n\tsw ra, " + std::to_string(-(FRAMESIZE + 4)) + "(sp)\n\tsw fp, " + std::to_string(-(FRAMESIZE + 8)) + "(sp)\n\tmv fp, sp\n\n";

    for (auto &p : statement->parameters) {
        p->symbol->location = ARG_REG;
        //functions_bff += genStmt(p);
    }

    for (const auto i : statement->body) {
        functions_bff += genStmt(i);
        text_bff.clear();
    }

    functions_bff += "\tlw ra, " + std::to_string(-(FRAMESIZE + 4)) + "(sp)\n\t" + "lw fp, " + std::to_string(-(FRAMESIZE + 8)) + "(sp)\n\t" + "addi sp, sp, " + std::to_string(-FRAMESIZE) + "\n\tret";
    functions_bff += "\n";
    return "";
}

std::string CodeGenerator::genRetStmt(const RetStmt *statement) {
    const Value rd = genExpr(statement->expression);

    functions_bff += text_bff;
    text_bff.clear();
    functions_bff += "\tmv a0, " + get_reg_name(rd) + "\n";
    t_register = 0; ////////

    return "\n";
}

std::string CodeGenerator::genFuncCallStmt(const FuncCallStmt *statement) {
    for (auto &arg : statement->arguments) {
        if (const auto *expr = dynamic_cast<VarExpr *>(arg)) {
            expr->symbol->location = TEMP_REG;
        }

        text_bff += "\tmv " + get_reg_name(Value{ARG_REG, arg_regs++}) + ", " + get_reg_name(genExpr(arg)) + "\n";
        t_register--;
    }

    text_bff += "\tcall " + statement->definition->id + "\n";

    return text_bff;
}
