#pragma once

#include <string>

namespace llp::similarity {

// Нормализация: убирает регистр, лишние пробелы, ASCII-пунктуацию.
// Unicode-символы (кириллица, японский) сохраняются как есть.
std::string normalize(const std::string& s);

// Количество кодовых точек в UTF-8 строке
std::size_t utf8_length(const std::string& s);

// Расстояние Левенштейна в кодовых точках
int levenshtein_distance(const std::string& a, const std::string& b);

// Нормализованное расстояние Левенштейна: 0..1 (1 = идентичны)
double levenshtein_score(const std::string& a, const std::string& b);

// Jaro-Winkler similarity: 0..1
double jaro_winkler(const std::string& a, const std::string& b);

// Комбинированная оценка: 0..1
// - exact match -> 1.0
// - иначе max(levenshtein_score, jaro_winkler)
double score(const std::string& a, const std::string& b);

}  // namespace llp::similarity