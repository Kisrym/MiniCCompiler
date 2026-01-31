#ifndef MINICCOMPILER_REGISTERPOOL_HPP
#define MINICCOMPILER_REGISTERPOOL_HPP

#include <utility>
#include <vector>
#include <string>

class RegisterPool {
    std::vector<bool> used;
    std::string prefix;

public:
    RegisterPool(const int n, std::string p)
        : used(n, false), prefix(std::move(p))
    {}

    int alloc();
    void free_reg(int r);
    std::string name(int r) const;
    void clear();
};

#endif //MINICCOMPILER_REGISTERPOOL_HPP