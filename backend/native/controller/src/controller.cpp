#include "llp/controller/controller.hpp"

#include <algorithm>

namespace llp::controller {

double apply_rate_limit(double I_new, double I_old, double m,
                        const ControllerParams& p) {
    // maturity: 1 для новой карточки, -> 0 для зрелой
    double maturity = 1.0 / (1.0 + m / p.m_ref);

    // rate_max: от (rate_floor + rate_span) для новой до rate_floor для зрелой
    double rate_max = p.rate_floor + p.rate_span * maturity;
    double rate_min = 1.0 / rate_max;

    double lower = rate_min * I_old;
    double upper = rate_max * I_old;

    return std::clamp(I_new, lower, upper);
}

StepResult step(const ControllerState& state,
                double score,
                double I_old,
                double I_pred,
                double m,
                const ControllerParams& p) {
    StepResult res;
    res.state = state;
    res.clamped = false;

    // Ошибка: score - R_target
    // score > R_target -> e > 0 -> интервал растёт
    // score < R_target -> e < 0 -> интервал уменьшается
    double e = score - p.R_target;

    // Интегральная составляющая
    double E_int_new = state.E_int + e;
    E_int_new = std::clamp(E_int_new, -p.E_max, p.E_max);

    // Сброс при lapse
    if (score < p.theta_lapse) {
        E_int_new = 0.0;
    }

    // Дифференциальная составляющая со сглаживанием
    double de_raw = e - state.e_prev;
    double de = p.gamma * de_raw + (1.0 - p.gamma) * state.de_prev;

    // Выход PID
    double u = p.Kp * e + p.Ki * E_int_new + p.Kd * de;

    // Сырой интервал
    double I_raw = I_pred * (1.0 + u);

    // Rate limiting
    double I_new = apply_rate_limit(I_raw, I_old, m, p);
    if (std::abs(I_new - I_raw) > 1e-9) {
        res.clamped = true;
    }

    // Физические пределы интервала
    I_new = std::clamp(I_new, p.I_min, p.I_max);

    // Anti-windup: если сработал clamp, откатываем интегратор
    if (res.clamped) {
        E_int_new = state.E_int;
    }

    res.state.E_int = E_int_new;
    res.state.e_prev = e;
    res.state.de_prev = de;
    res.u = u;
    res.I_new = I_new;
    return res;
}

}  // namespace llp::controller