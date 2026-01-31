#include "codegenerator.hpp"

#include "argreg.hpp"
#include "tempreg.hpp"

CodeGenerator::CodeGenerator(Parser *parser, SemanticAnalyzer *semanticAnalyzer)
    : parser(parser), analyzer(semanticAnalyzer), temps_pool(RegisterPool(7, "t")), args_pool(RegisterPool(8, "a")) {
    while (Stmt *current_statement = parser->parseStatement()) {
        instructions.push_back(analyzer->analyze(current_statement));
        // armazena as instruções com as anotações do semantic analyzer
    }

    current_instruction = instructions[0];
    FRAME_SIZE = analyzer->get_current_offset(); - 4;
}

std::vector<std::string> CodeGenerator::generateCode() {
    // settar o prologo do programa (configuração de sp e fp)
    int FRAMESIZE = 16 * (((FRAME_SIZE) + 15) / 16);
    const std::string fs_str = std::to_string(-FRAMESIZE);
    const std::string fs4_str = std::to_string(FRAMESIZE - 16);

    data_bff.append(".data\n");

    generated_code.emplace_back(".text\n");
    generated_code.push_back("\taddi sp, sp, " + fs_str + "\n\tsw fp, " + fs4_str + "(sp)""\n\tmv fp, sp" + "\n\n");

    for (Stmt *stmt : instructions) {
        genStmt(stmt);
        generated_code.push_back(text_bff);
        text_bff.clear();
    }

    generated_code.push_back("\n\tlw fp, " + fs4_str + "(sp)\n\taddi sp, sp, " + fs_str.substr(1) + "\n");
    generated_code.emplace_back("\tli a7, 10\n\tecall\n"); // exit program rars

    generated_code.push_back(functions_bff);

    generated_code.insert(generated_code.cbegin(), data_bff);

    return generated_code;
}
/*
std::string CodeGenerator::load(const value &v) {
    const TempReg reg(temps_pool);
    if (v.storage == Storage::STACK) {
        return "lw " + temps_pool.name(reg.r) + ", " + std::to_string(v.offset) + "(fp)";
        //return "AQUI";
    }

    // se for temporario
    //return "mv " + get_reg_name(Value(TEMP_REG, t_register)) + ", t" + std::to_string(v.index);
    return "mv " + temps_pool.name(reg.r) + ", " + temps_pool.name(v.reg);
}
*/
// expressions
value CodeGenerator::genExpr(Expr *expression) {
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

    return value{Storage::REGISTER, -1};
}

value CodeGenerator::genExpr(Expr *expression, const std::string &label_else, const std::string &label_if) {
    if (const auto expr = dynamic_cast<BinaryExpr *>(expression)) {
        return genBinaryExpr(expr, label_else, label_if);
    }

    return genExpr(expression);
}

value CodeGenerator::genLiteral(const IntExpr *expr) {
    const TempReg reg(temps_pool);
    const value v(Storage::REGISTER, reg.r);

    text_bff += "\tli " + temps_pool.name(reg.r) + ", " + std::to_string(expr->value) + "\n";

    return v;
}

value CodeGenerator::genBoolExpr(const BoolExpr *expr) {
    const TempReg reg(temps_pool);
    const value v(Storage::REGISTER, reg.r);

    if (const unsigned int conversion = expr->value) {
        text_bff += "\tli " + temps_pool.name(reg.r) + ", " + std::to_string(conversion) + "\n";

        return v;
    }

    return value{Storage::REGISTER, -999}; // vale x0 (zero)
}

value CodeGenerator::genStringExpr(const StringExpr *expr) {
    const TempReg reg(temps_pool);
    const value v(Storage::REGISTER, reg.r);
    const std::string string_label("STRING" + std::to_string(l_string));

    data_bff.append("\t"+ string_label + ": .asciz " + "\"" + expr->value +"\"\n");
    text_bff.append("\tla " + temps_pool.name(reg.r) + ", " + string_label + "\n");

    l_string++;

    return v;
}

value CodeGenerator::genVarExpr(const VarExpr *expr) {
    //if (expr->symbol->location == ARG_REG) {
    //    return Value{ARG_REG, arg_regs++};
    //}

    const TempReg reg(temps_pool);
    const auto v = value{expr->symbol->location, reg.r};

    text_bff += "\tlw " + temps_pool.name(reg.r) + ", " + std::to_string(expr->symbol->stackOffset) + "(fp)\n";

    return v;
}

value CodeGenerator::genBinaryExpr(const BinaryExpr *expr) {
    const TempReg leftReg(temps_pool);
    const TempReg rightReg(temps_pool);

    const value left  = genExpr(expr->value1);
    text_bff += "\tmv " + temps_pool.name(leftReg.r) + ", " + temps_pool.name(left.reg) + "\n";

    const value right = genExpr(expr->value2);
    text_bff += "\tmv " + temps_pool.name(rightReg.r) + ", " + temps_pool.name(right.reg) + "\n";

    const std::string rd = temps_pool.name(leftReg.r);
    const std::string rs = temps_pool.name(rightReg.r);
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

    if (op == GREATER || op == LE) {
        text_bff += "\t" + command + rd + ", " + rs + ", " + rd + "\n"; // inverte a fim de simular comparações de maior que

        if (op == LE) {
            text_bff += "\txori " + rd + ", " + rd + ", 1\n";
        }
    }

    else {
        text_bff += "\t" + command + rd + ", " + rd + ", " + rs + "\n";

        if (op == EQUAL) {
            text_bff += "\tsltiu " + rd + ", " + rd + ", 1\n"; // (xor x, y == 0) ? x == y : x != y. basicamente, essa instrução verifica isso
        }
        else if (op == NEQUAL) {
            text_bff += "\tsltu " + rd + ", zero, " + rd + "\n";
        }
    }

    return value{Storage::REGISTER, leftReg.r};
}

value CodeGenerator::genBinaryExpr(const BinaryExpr *expr, const std::string &label_else, const std::string &label_if) {
    if (expr->op.type == AND || expr->op.type == OR) {
        op_stack.push(expr->op.type);
    }
    const TempReg leftReg(temps_pool);
    const TempReg rightReg(temps_pool);

    const value left  = genExpr(expr->value1);
    text_bff += "\tmv " + temps_pool.name(leftReg.r) + ", " + temps_pool.name(left.reg) + "\n";

    const value right = genExpr(expr->value2);
    text_bff += "\tmv " + temps_pool.name(rightReg.r) + ", " + temps_pool.name(right.reg) + "\n";

    const std::string rd = temps_pool.name(leftReg.r);
    const std::string rs = temps_pool.name(rightReg.r);

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

    // a forma como serão feitos os saltos simularão o AND e OR
    // se ambos pularem pro ELSE, será um AND, se pularem pro IF, será um OR
    if (!op_stack.empty() && op_stack.top() == OR){
        text_bff += "\t" + command + rd + ", " + rs + ", " + label_if + "\n";
    }

    else if (!command.empty()){
        text_bff += "\t" + command + rd + ", " + rs + ", " + label_else + "\n";
    }

    if (!op_stack.empty())
        op_stack.pop();

    return value{Storage::REGISTER, left.reg};
}

value CodeGenerator::genUnaryExpr(const UnaryExpr *expr) {
    const TokenType op = expr->op.type;

    const value rd = genExpr(expr->value1);
    const std::string rd_name = temps_pool.name(rd.reg);

    if (op == MINUS) {
        text_bff += "\tsub " + rd_name + "zero, " + rd_name + "\n";

        return rd;
    }

    // caso default: NOT
    text_bff += "\txori " + rd_name + ", " + rd_name + ", -1\n";

    return rd;
}

value CodeGenerator::genFuncCallExpr(const FuncCallExpr *expr) {
    for (auto &arg : expr->arguments) {
        const int arg_reg = args_pool.alloc();

        text_bff += "\tmv " + args_pool.name(arg_reg) + ", " + temps_pool.name(genExpr(arg).reg) + "\n";
    }

    args_pool.clear();

    const TempReg reg(temps_pool);
    text_bff += "\tcall " + expr->definition->id + "\n\tmv " + temps_pool.name(reg.r) + ", a0\n";

    return value{Storage::REGISTER, reg.r}; // MODIFICAR TALVEZ PARA RETORNO DE 2 VARIAVEIS (a0, a1) => POSSIVELMENTE NAO
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
    text_bff += "\tsw " + temps_pool.name(value.reg) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";

    return text_bff;
}

std::string CodeGenerator::genAssign(const AssignStmt *statement) {
    const TempReg reg(temps_pool);
    const value v(Storage::REGISTER, reg.r);

    const std::string rd = temps_pool.name(v.reg);

    // carrega a variavel pro assign
    //text_bff += "\t" + load(value{Storage::STACK, statement->symbol->stackOffset}) + "\n";
    //text_bff += "\tlw " + get_reg_name(t_register) + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n";

    const std::string rs1 = temps_pool.name(genExpr(statement->expression).reg); // armazena o reg com o resultado da expressão em rs1

    text_bff += "\tadd " + rd + ", zero, " + rs1 + "\n"; // OTIMIZAÇÃO DPS
    text_bff += "\tsw " + rd + ", " + std::to_string(statement->symbol->stackOffset) + "(fp)\n"; // salva dnv

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
    FRAMESIZE = 16 * (((FRAMESIZE + 4) + 15) / 16); // 4 para armazenar ra e modulo para manter alinhamento de memoria
    FRAMESIZE = -FRAMESIZE;

    functions_bff += "\taddi sp, sp, " + std::to_string(FRAMESIZE) + "\n\tsw ra, 0(sp)\n\tsw fp, 4(sp)\n\tmv fp, sp" + "\n\n";

    int arg = 0;
    for (auto &p : statement->parameters) {
        p->symbol->location = Storage::STACK;
        functions_bff += "sw a" + std::to_string(arg++) + ", " + std::to_string(p->symbol->stackOffset) + "(fp)\n";
    }

    for (const auto i : statement->body) {
        functions_bff += genStmt(i);
        text_bff.clear();
    }

    functions_bff += "\tlw ra, 0(fp)\n\tlw fp, 4(fp)\n\taddi sp, sp, " + std::to_string(-FRAMESIZE) + "\n\tret";
    functions_bff += "\n";
    return "";
}

std::string CodeGenerator::genRetStmt(const RetStmt *statement) {
    const value rd = genExpr(statement->expression);

    functions_bff += text_bff;
    text_bff.clear();
    functions_bff += "\tmv a0, " + temps_pool.name(rd.reg) + "\n";

    return "\n";
}

std::string CodeGenerator::genFuncCallStmt(const FuncCallStmt *statement) {
    for (auto &arg : statement->arguments) {
        const int arg_reg = args_pool.alloc();

        text_bff += "\tmv " + args_pool.name(arg_reg) + ", " + temps_pool.name(genExpr(arg).reg) + "\n";
    }

    args_pool.clear();

    text_bff += "\tcall " + statement->definition->id + "\n";

    return text_bff;
}
