#include "llp/memory/memory.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace llp::memory;

void test_recall_at_t_zero() {
    assert(std::abs(recall(0.0, 0.0, 1.0, 0.5) - 1.0) < 1e-9);
    std::cout << "[OK] recall(0) = 1.0\n";
}

void test_recall_decays() {
    double r1 = recall(1.0, 0.0, 1.0, 0.5);
    double r4 = recall(4.0, 0.0, 1.0, 0.5);
    assert(r1 > r4);
    std::cout << "[OK] recall(1)=" << r1 << " > recall(4)=" << r4 << "\n";
}

void test_predict_interval() {
    // R(t) = t^(-0.5), R_target = 0.9
    // I = (0.9)^(-1/0.5) = 0.9^(-2) ≈ 1.2346
    double I = predict_interval(0.0, 1.0, 0.5, 0.9, 1.0, 365.0);
    assert(std::abs(I - 1.2346) < 0.01);
    std::cout << "[OK] predict_interval = " << I << "\n";
}

void test_update_strength_success() {
    MemoryParams p;
    double m0 = 0.0;
    double m1 = update_strength(m0, 1.0, 1000.0, p);
    assert(m1 > m0);
    std::cout << "[OK] update_strength (success) " << m0 << " -> " << m1 << "\n";
}

void test_update_strength_fail() {
    MemoryParams p;
    double m0 = 5.0;
    double m1 = update_strength(m0, 0.0, 3000.0, p);
    assert(m1 < m0);
    std::cout << "[OK] update_strength (fail) " << m0 << " -> " << m1 << "\n";
}

int main() {
    test_recall_at_t_zero();
    test_recall_decays();
    test_predict_interval();
    test_update_strength_success();
    test_update_strength_fail();
    std::cout << "\nAll memory tests passed.\n";
    return 0;
}