#pragma once

#include <cmath>

namespace llp::controller {

struct ControllerState {
    double E_int = 0.0;
    double e_prev = 0.0;
    double de_prev = 0.0;
};

struct ControllerParams {
    // PID
    double Kp = 0.5;
    double Ki = 0.1;
    double Kd = 0.05;
    double gamma = 0.3;          // сглаживание D-члена

    // Anti-windup и сброс
    double E_max = 10.0;         // предел интегратора
    double theta_lapse = 0.3;    // ниже — сброс E_int

    // Уставка
    double R_target = 0.9;

    // Rate limiting (gain scheduling)
    double rate_span = 2.0;      // для новых карточек: rate_max = 1 + span
    double rate_floor = 1.0;     // для зрелых: rate_max стремится к floor
    double m_ref = 5.0;          // референс maturity

    // Физические пределы интервала
    double I_min = 1.0;
    double I_max = 365.0;
};

struct StepResult {
    ControllerState state;
    double u;                    // выход PID
    double I_new;                // итоговый интервал
    bool clamped;                // сработал ли rate limiter
};

// Rate limiting с gain scheduling: границы зависят от прочности m
double apply_rate_limit(double I_new, double I_old, double m,
                        const ControllerParams& p);

// Один шаг PID
StepResult step(const ControllerState& state,
                double score,
                double I_old,
                double I_pred,
                double m,
                const ControllerParams& p);

}  // namespace llp::controller