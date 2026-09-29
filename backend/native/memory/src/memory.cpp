#include "llp/memory/memory.hpp"

#include <algorithm>
#include <cmath>

namespace llp::memory {

double recall(double t, double a, double b, double c) {
    if (t <= 0.0) return std::clamp(a + b, 0.0, 1.0);
    double value = a + b * std::pow(t, -c);
    return std::clamp(value, 0.0, 1.0);
}

double predict_interval(double a, double b, double c, double R_target,
                        double I_min, double I_max) {
    // I_pred = ((R_target - a) / b)^(-1/c)
    double num = R_target - a;
    if (num <= 0.0) {
        // Цель ниже асимптоты — забывание не достигнет цели
        return I_max;
    }
    double ratio = num / b;
    double I = std::pow(ratio, -1.0 / c);
    return std::clamp(I, I_min, I_max);
}

double update_strength(double m, double score, double tau_norm,
                       const MemoryParams& p) {
    // Фактор скорости: быстрый ответ -> ближе к 1
    double f_speed = std::exp(-tau_norm / p.T_0);

    // Фактор насыщения: чем прочнее память, тем труднее укрепить
    double f_sat = 1.0 / (1.0 + m / p.m_sat);

    // Приращение при успехе
    double dm_plus = p.eta_plus * score * f_speed * f_sat;

    // Приращение при провале: чем прочнее была память, тем больнее
    double dm_minus = -p.eta_minus * (1.0 - score) * (1.0 + m / p.m_ref);

    // Плавное смешивание по score
    double dm = score * dm_plus + (1.0 - score) * dm_minus;

    return std::clamp(m + dm, 0.0, p.m_max);
}

MemoryState update_state(const MemoryState& state,
                         double score,
                         double t_response,
                         double answer_length,
                         const MemoryParams& p) {
    MemoryState result = state;

    // Нормализация времени ответа
    double len = std::max(answer_length, 1.0);
    double tau_norm = t_response / len;

    // Обновление прочности
    result.m = update_strength(state.m, score, tau_norm, p);

    return result;
}

}  // namespace llp::memory