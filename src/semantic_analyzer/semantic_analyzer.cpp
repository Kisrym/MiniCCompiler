#include  "semantic_analyzer.hpp"

#include <bits/locale_facets_nonio.h>

bool SemanticAnalyzer::is_in_scope(const std::string &id) const {
    for (int i = scopes.size()-1; i >= 0; --i) {
        if (scopes[i].contains(id)) {
            return true;
        }
    }
    return false;
}

TokenType SemanticAnalyzer::analyze(Expr *expr) {
    if (const auto expression = dynamic_cast<VarExpr *>(expr)) { // variaveis
        const std::string id = expression->value;

        if (!is_in_scope(id)) {
            throw std::runtime_error("Variable '" + id + "' not declared in this scope.");
        }

        auto *symb = new Symbol();

        for (int i = scopes.size()-1; i >= 0; --i) { // procura no escopo atual e nos acima
            if (scopes[i].contains(id)) {
                symb = &scopes[i][id];
                break;
            }
        }

        expression->symbol = symb;
        return scopes.back()[id].type;
    }

    if (const auto _ = dynamic_cast<IntExpr *>(expr)) {
        return INT;
    }

    if (const auto _ = dynamic_cast<DoubleExpr *>(expr)) {
        return DOUBLE;
    }

    if (const auto _ = dynamic_cast<BoolExpr *>(expr)) {
        return BOOL;
    }

    if (const auto expression = dynamic_cast<BinaryExpr *>(expr)) {
        const TokenType type_v1 = analyze(expression->value1);
        const TokenType type_v2 = analyze(expression->value2);
        const TokenType op = expression->op.type;

        // OTIMIZAÇÃO AQUI DEPOIS: TALVEZ SEPARAR TIPOS DE DADOS ENTRE NUM E BOOL, EVITANDO TANTA COMPARAÇÃO
        if (op != LESSER && op != GREATER && op != LE && op != GE && op != EQUAL && op != NEQUAL && op != AND && op != OR) {
            if ((type_v1 == DOUBLE || type_v2 == DOUBLE) && (type_v1 == INT || type_v2 == INT)) {
                expression->inferredType = DOUBLE;
                return DOUBLE;
            }

            if (type_v1 != type_v2) { // por enquanto vai ser geral, posteriormente tipos vao se sobressair
                throw std::runtime_error("Incompatible types.");
            }

            expression->inferredType = type_v1;
            return type_v1; // se forem iguais
        }
        // expressões booleanas
        else {
            if ((type_v1 == STRING || type_v2 == STRING) && (type_v1 != STRING || type_v2 != STRING)) {
                throw std::runtime_error("Comparison between incomparable types.");
            }

            expression->inferredType = BOOL;
            return BOOL;
        }
    }

    if (const auto expression = dynamic_cast<UnaryExpr *>(expr)) {
        const TokenType type_v1 = analyze(expression->value1);
        const TokenType op = expression->op.type;

        if (type_v1 == STRING && op == MINUS) { // um exemplo apenas, tera mais (DEPOIS ADICIONAR ERROS RELACIONADOS AO NOT)
            throw std::runtime_error("Incompatible types.");
        }

        if (op == NOT) {
            expression->inferredType = BOOL;
            return BOOL;
        }

        expression->inferredType = type_v1;
        return type_v1;
    }

    throw std::runtime_error("Invalid expression");
}
Stmt *SemanticAnalyzer::analyze(Stmt *stmt) {
    if (const auto statement = dynamic_cast<VarDeclStmt *>(stmt)) {
        auto *symbol = new Symbol(statement->type.type, &current_offset);

        if (scopes.back().empty()) {
            const TokenType expr_type = analyze(statement->expression);
            const TokenType stmt_type = statement->type.type;

            if ((stmt_type == INT && expr_type == DOUBLE)
            ||  (stmt_type == STRING && (expr_type == INT || expr_type == DOUBLE))
            ||  ((stmt_type == INT || stmt_type == DOUBLE) && expr_type == STRING)) { // adicionar o tratamento para tipo booleano posteriormente
                throw std::runtime_error("Incompatible types.");
            }

            // faz sentido? tirar?
            if (stmt_type == INT && expr_type == BOOL) {
                symbol->type = BOOL; // conversao implicita
                symbol->stackOffset = -1;
            }

            statement->symbol = symbol;
            scopes.back()[statement->id] = *symbol;
        }
        else {
            analyze(statement->expression);
            if (scopes.back().contains(statement->id)) {
                throw std::runtime_error("Variable '" + statement->id + "' already declared in this scope.");
            }

            statement->symbol = symbol;
            scopes.back()[statement->id] = *symbol;
        }

        return statement;
    }

    if (const auto statement = dynamic_cast<AssignStmt *>(stmt)) {
        if (!is_in_scope(statement->id)) {
            throw std::runtime_error("Variable '" + statement->id + "' not declared in this scope.");
        }

        const TokenType expr_type = analyze(statement->expression);
        TokenType stmt_type = UNKNOWN;
        auto *symb = new Symbol();

        for (int i = scopes.size()-1; i >= 0; --i) { // procura no escopo atual e nos acima
            if (scopes[i].contains(statement->id)) {
                stmt_type = scopes[i][statement->id].type;
                symb = &scopes[i][statement->id];
            }
        }

        if (stmt_type == DOUBLE && (expr_type == INT || expr_type == DOUBLE)) {
            statement->symbol = symb; // NAO MODIFICA O OFFSET
            return statement;
        }

        if (stmt_type == expr_type) {
            statement->symbol = symb; // NAO MODIFICA O OFFSET
            return statement;
        }

        throw std::runtime_error("Incompatible types.");

        // adicionar mais conversões implicitas, como char pra string, por exemplo
        // caso contrario, tem nada de errado
    }

    if (const auto statement = dynamic_cast<IfStmt *>(stmt)) {
        SymbolTable symbol_table;

        analyze(statement->condition); // condicao ainda fica no escopo anterior

        scopes.push_back(symbol_table);

        for (const auto &branch_st : statement->thenBranches) {
            analyze(branch_st);
        }

        scopes.back().clear(); // acabou o escopo do if

        for (const auto &branch_st : statement->elseBranches) {
            analyze(branch_st);
        }

        scopes.pop_back(); // não teve nenhum erro de escopo nesse escopo
        return statement;
    }

    if (const auto statement = dynamic_cast<ForStmt *>(stmt)) {
        SymbolTable symbol_table;

        // ainda ficam no escopo anterior
        analyze(statement->definition);
        analyze(statement->condition);
        analyze(statement->increment);

        scopes.push_back(symbol_table);
        for (const auto &body_stmt : statement->body) {
            analyze(body_stmt);
        }

        scopes.pop_back();
        return statement;
    }

    return nullptr;
}