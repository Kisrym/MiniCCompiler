#ifndef MINICCOMPILER_TEMPREG_HPP
#define MINICCOMPILER_TEMPREG_HPP

#include "registerpool.hpp"

class TempReg {
public:
    RegisterPool &pool;
    int r;

    TempReg(RegisterPool &pool)
        : pool(pool), r(pool.alloc())
    {}
    ~TempReg() {pool.free_reg(r);}
};

#endif //MINICCOMPILER_TEMPREG_HPP