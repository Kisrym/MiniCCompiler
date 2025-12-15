#ifndef MINICCOMPILER_SEMANTIC_ANALYZER_HPP
#define MINICCOMPILER_SEMANTIC_ANALYZER_HPP

#include <string>
#include <token.hpp>
#include <unordered_map>

#include "stmt.hpp"
#include "expr.hpp"

using SymbolTable = std::unordered_map<std::string, Symbol>;

class SemanticAnalyzer {
private:
    std::vector<SymbolTable> scopes;
    bool is_in_scope(const std::string &id) const;
    int current_offset = 0;

public:
    TokenType analyze(Expr *expr);
    Stmt *analyze(Stmt *stmt);
    int get_current_offset() const {
        return current_offset;
    };

    SemanticAnalyzer() {
        SymbolTable symbol_table;
        scopes.push_back(symbol_table);
    }

};

#endif //MINICCOMPILER_SEMANTIC_ANALYZER_HPP