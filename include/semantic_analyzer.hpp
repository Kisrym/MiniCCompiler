#ifndef MINICCOMPILER_SEMANTIC_ANALYZER_HPP
#define MINICCOMPILER_SEMANTIC_ANALYZER_HPP

#include <stack>
#include <string>
#include <token.hpp>
#include <unordered_map>

#include "stmt.hpp"
#include "expr.hpp"

using SymbolTable = std::unordered_map<std::string, Symbol*>;

class SemanticAnalyzer {
private:
    std::vector<SymbolTable> scopes;
    bool is_in_scope(const std::string &id) const;
    int current_offset = 0;
    std::stack<int> offset;

public:
    TokenType analyze(Expr *expr);
    Stmt *analyze(Stmt *stmt);
    int get_current_offset() const {
        return offset.top();
    };

    SemanticAnalyzer() {
        SymbolTable symbol_table;
        offset.push(0);
        scopes.push_back(symbol_table);
    }

};

#endif //MINICCOMPILER_SEMANTIC_ANALYZER_HPP