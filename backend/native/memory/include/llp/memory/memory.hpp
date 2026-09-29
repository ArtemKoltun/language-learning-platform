#pragma once

#include <cmath>
#include <stdexcept>

namespace llp::memory {

// Состояние памяти одной карточки
struct MemoryState {
    double m = 0.0;           // прочность памяти (безразмерная)
    double t_last = 0.0;      // время последнего повторения (дни)
    double E_int = 0.0;       // интеграл ошибки PID
    double e_prev = 0.0;      // предыдущая ошибка (для D)
    double de_prev = 0.0;     // предыдущая производная (для фильтра)
};

// Параметры модели памяти
struct MemoryParams {
    // Кривая забывания: R(t) = a + b * t^(-c)
    double a = 0.0;           // асимптота [0, 1)
    double b = 1.0;           // начальный уровень (> 0)
    double c = 0.5;           // скорость забывания (> 0)

    // Целевой retention
    double R_target = 0.9;

    // Обновление прочности
    double eta_plus = 1.0;    // скорость обучения
    double eta_minus = 1.5;   // скорость забывания
    double m_sat = 10.0;      // насыщение при успехе
    double m_ref = 5.0;       // референс при провале
    double T_0 = 2000.0;      // характерное время ответа (мс/символ)

    // Ограничения
    double I_min = 1.0;
    double I_max = 365.0;
    double m_max = 50.0;
};

// Вероятность вспомнить через время t
double recall(double t, double a, double b, double c);

// Предсказанный интервал при целевом retention
double predict_interval(double a, double b, double c, double R_target,
                        double I_min, double I_max);

// Обновление прочности памяти
double update_strength(double m, double score, double tau_norm,
                       const MemoryParams& p);

// Обновление полного состояния после ответа
MemoryState update_state(const MemoryState& state,
                         double score,
                         double t_response,
                         double answer_length,
                         const MemoryParams& p);

}  // namespace llp::memory