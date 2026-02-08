#ifndef MINICCOMPILER_IRBUILDER_HPP
#define MINICCOMPILER_IRBUILDER_HPP

#include <vector>

#include "expr.hpp"
#include "irinstr.hpp"
#include "stmt.hpp"

class IRBuilder {
    std::vector<IRInstr> ir;
    int t_register = -1;
    int new_register() {return ++t_register;}

    void genStmt(Stmt *stmt);
    int genExpr(Expr *expr);
};

#endif //MINICCOMPILER_IRBUILDER_HPP