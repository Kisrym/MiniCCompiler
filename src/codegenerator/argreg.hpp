#ifndef MINICCOMPILER_ARGREG_HPP
#define MINICCOMPILER_ARGREG_HPP

#include "registerpool.hpp"

class ArgReg {
public:
    RegisterPool &pool;
    int r;

    ArgReg(RegisterPool &pool)
        : pool(pool), r(pool.alloc())
    {}
    ~ArgReg() {pool.free_reg(r);}
};

#endif //MINICCOMPILER_ARGREG_HPP