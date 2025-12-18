#ifndef __AST_NODE_H__
#define __AST_NODE_H__

#include "token.hpp"

class ASTNode {
public:
    virtual ~ASTNode() = default; // chama o destrutor da subclasse corretamente
};

#endif