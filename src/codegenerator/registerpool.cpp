#include "registerpool.hpp"

#include <stdexcept>

int RegisterPool::alloc() {
    for (int i = 0; i < used.size(); i++) {
        if (!used[i]) {
            used[i] = true;
            return i;
        }
    }

    throw std::runtime_error("Out of registers");
}

void RegisterPool::free_reg(const int r) {
    used[r] = false;
}

std::string RegisterPool::name(const int r) const {
    return prefix + std::to_string(r);
}

void RegisterPool::clear() {
    for (auto && i : used) {
        i = false;
    }
}
