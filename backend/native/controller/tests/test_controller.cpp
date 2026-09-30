#include "llp/controller/controller.hpp"
#include "llp/memory/memory.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace llp::controller;
using llp::memory::MemoryParams;

void test_rate_limit_new_card() {
    ControllerParams p;
    double I = apply_rate_limit(100.0, 10.0, 0.0, p);
    std::cout << "[OK] new card: I_old=10 -> I_new=" << I << "\n";
    assert(std::abs(I - 30.0) < 1e-9);
}

void test_rate_limit_mature_card() {
    ControllerParams p;
    double I = apply_rate_limit(100.0, 10.0, 100.0, p);
    std::cout << "[OK] mature card: I_old=10 -> I_new=" << I << "\n";
    assert(I < 11.0 && I > 9.0);
}

void test_rate_limit_min_direction() {
    ControllerParams p;
    double I = apply_rate_limit(1.0, 30.0, 0.0, p);
    std::cout << "[OK] new card down: I_old=30 -> I_new=" << I << "\n";
    assert(std::abs(I - 10.0) < 1e-9);
}

void test_pid_success() {
    ControllerParams p;
    ControllerState s;
    auto r = step(s, 1.0, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] success: I_old=10 -> I_new=" << r.I_new
              << ", u=" << r.u << "\n";
    assert(r.I_new > 10.0);
}

void test_pid_fail() {
    ControllerParams p;
    ControllerState s;
    auto r = step(s, 0.0, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] fail: I_old=10 -> I_new=" << r.I_new
              << ", u=" << r.u << "\n";
    assert(r.I_new < 10.0);
}

void test_pid_neutral() {
    ControllerParams p;
    ControllerState s;
    auto r = step(s, p.R_target, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] neutral: I_new=" << r.I_new << "\n";
    assert(std::abs(r.I_new - 10.0) < 1e-9);
}

void test_lapse_reset() {
    ControllerParams p;
    ControllerState s;
    s.E_int = 5.0;
    auto r = step(s, 0.1, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] lapse: E_int=" << r.state.E_int << "\n";
    assert(r.state.E_int == 0.0);
}

void test_anti_windup() {
    ControllerParams p;
    ControllerState s;
    for (int i = 0; i < 100; ++i) {
        auto r = step(s, 1.0, 10.0, 10.0, 0.0, p);
        s = r.state;
    }
    std::cout << "[OK] anti-windup: E_int=" << s.E_int << "\n";
    assert(std::abs(s.E_int) <= p.E_max);
}

void test_regulate_step_success() {
    ControllerParams cp;
    MemoryParams mp;
    mp.a = 0.0;
    mp.b = 1.0;
    mp.c = 0.5;
    mp.R_target = 0.9;
    mp.I_min = 1.0;
    mp.I_max = 365.0;

    ControllerState cs;
    double m = 0.0;
    double I_old = 1.0;

    auto r = regulate_step(cs, m, 1.0, 1000.0, 5.0, I_old, mp, cp);

    std::cout << "[OK] regulate success: I_pred=" << r.I_pred
              << ", I_new=" << r.I_new
              << ", m_new=" << r.m_new
              << ", u=" << r.u << "\n";

    assert(r.m_new > m);
    assert(r.I_pred > 1.0);
    assert(r.I_new > 1.0);
}

void test_regulate_step_fail() {
    ControllerParams cp;
    MemoryParams mp;
    mp.a = 0.0;
    mp.b = 1.0;
    mp.c = 0.5;
    mp.R_target = 0.9;
    mp.I_min = 1.0;
    mp.I_max = 365.0;

    ControllerState cs;
    double m = 5.0;
    double I_old = 10.0;

    auto r = regulate_step(cs, m, 0.0, 3000.0, 5.0, I_old, mp, cp);

    std::cout << "[OK] regulate fail: I_pred=" << r.I_pred
              << ", I_new=" << r.I_new
              << ", m_new=" << r.m_new
              << ", u=" << r.u << "\n";

    assert(r.m_new < m);
    assert(r.I_new < I_old);
}

void test_regulate_step_neutral() {
    ControllerParams cp;
    MemoryParams mp;
    mp.a = 0.0;
    mp.b = 1.0;
    mp.c = 0.5;
    mp.R_target = 0.9;
    mp.I_min = 1.0;
    mp.I_max = 365.0;

    ControllerState cs;
    auto r = regulate_step(cs, 0.0, 0.9, 1000.0, 5.0, 1.2346, mp, cp);

    std::cout << "[OK] regulate neutral: I_pred=" << r.I_pred
              << ", I_new=" << r.I_new
              << ", u=" << r.u << "\n";

    assert(std::abs(r.I_new - r.I_pred) < 1e-6);
}

int main() {
    test_rate_limit_new_card();
    test_rate_limit_mature_card();
    test_rate_limit_min_direction();
    test_pid_success();
    test_pid_fail();
    test_pid_neutral();
    test_lapse_reset();
    test_anti_windup();
    test_regulate_step_success();
    test_regulate_step_fail();
    test_regulate_step_neutral();
    std::cout << "\nAll controller tests passed.\n";
    return 0;
}