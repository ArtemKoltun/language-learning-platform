#include "llp/controller/controller.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace llp::controller;

void test_rate_limit_new_card() {
    ControllerParams p;
    // Новая карточка: m = 0, maturity = 1
    // rate_max = 1 + 2 = 3, rate_min = 1/3
    double I = apply_rate_limit(100.0, 10.0, 0.0, p);
    std::cout << "[OK] new card: I_old=10 -> I_new=" << I << "\n";
    assert(std::abs(I - 30.0) < 1e-9);
}

void test_rate_limit_mature_card() {
    ControllerParams p;
    // Зрелая карточка: m большое, maturity -> 0
    double I = apply_rate_limit(100.0, 10.0, 100.0, p);
    std::cout << "[OK] mature card: I_old=10 -> I_new=" << I << "\n";
    assert(I < 11.0 && I > 9.0);
}

void test_rate_limit_min_direction() {
    ControllerParams p;
    // Новая карточка, I_new пытается упасть сильно
    double I = apply_rate_limit(1.0, 30.0, 0.0, p);
    std::cout << "[OK] new card down: I_old=30 -> I_new=" << I << "\n";
    assert(std::abs(I - 10.0) < 1e-9);
}

void test_pid_success() {
    ControllerParams p;
    ControllerState s;
    // score=1.0 -> e=+0.1, интервал растёт
    auto r = step(s, 1.0, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] success: I_old=10 -> I_new=" << r.I_new
              << ", u=" << r.u << "\n";
    assert(r.I_new > 10.0);
}

void test_pid_fail() {
    ControllerParams p;
    ControllerState s;
    // score=0.0 -> e=-0.9, интервал уменьшается
    auto r = step(s, 0.0, 10.0, 10.0, 5.0, p);
    std::cout << "[OK] fail: I_old=10 -> I_new=" << r.I_new
              << ", u=" << r.u << "\n";
    assert(r.I_new < 10.0);
}

void test_pid_neutral() {
    ControllerParams p;
    ControllerState s;
    // score = R_target -> e = 0, u = 0, интервал = I_pred
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
    // Прогоняем много успешных ответов, интегратор не должен улететь
    for (int i = 0; i < 100; ++i) {
        auto r = step(s, 1.0, 10.0, 10.0, 0.0, p);
        s = r.state;
    }
    std::cout << "[OK] anti-windup: E_int=" << s.E_int << "\n";
    assert(std::abs(s.E_int) <= p.E_max);
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
    std::cout << "\nAll controller tests passed.\n";
    return 0;
}