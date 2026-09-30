#include "llp/similarity/similarity.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <vector>

namespace llp::similarity {

namespace {

// Декодирует UTF-8 строку в вектор кодовых точек.
// Невалидные байты пропускаются.
std::vector<char32_t> utf8_decode(const std::string& s) {
    std::vector<char32_t> out;
    out.reserve(s.size());
    std::size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        char32_t cp = 0;
        int len = 0;
        if (c < 0x80)      { cp = c;          len = 1; }
        else if ((c & 0xE0) == 0xC0) { cp = c & 0x1F; len = 2; }
        else if ((c & 0xF0) == 0xE0) { cp = c & 0x0F; len = 3; }
        else if ((c & 0xF8) == 0xF0) { cp = c & 0x07; len = 4; }
        else { ++i; continue; }  // невалидный байт

        if (i + len > s.size()) break;

        for (int j = 1; j < len; ++j) {
            cp = (cp << 6) | (static_cast<unsigned char>(s[i + j]) & 0x3F);
        }
        out.push_back(cp);
        i += len;
    }
    return out;
}

// Возвращает длину UTF-8 последовательности по первому байту.
// 0 = невалидный байт.
int utf8_char_len(unsigned char c) {
    if (c < 0x80) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 0;
}

int levenshtein_distance_cp(const std::vector<char32_t>& a,
                            const std::vector<char32_t>& b) {
    const std::size_t n = a.size();
    const std::size_t m = b.size();
    if (n == 0) return static_cast<int>(m);
    if (m == 0) return static_cast<int>(n);

    // Работаем с двумя строками DP, O(min(n,m)) памяти
    std::vector<int> prev(m + 1);
    std::vector<int> curr(m + 1);
    for (std::size_t j = 0; j <= m; ++j) prev[j] = static_cast<int>(j);

    for (std::size_t i = 1; i <= n; ++i) {
        curr[0] = static_cast<int>(i);
        for (std::size_t j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            curr[j] = std::min({
                prev[j] + 1,       // удаление
                curr[j - 1] + 1,   // вставка
                prev[j - 1] + cost // замена
            });
        }
        std::swap(prev, curr);
    }
    return prev[m];
}

double jaro_cp(const std::vector<char32_t>& s1,
               const std::vector<char32_t>& s2) {
    if (s1.empty() && s2.empty()) return 1.0;
    if (s1.empty() || s2.empty()) return 0.0;

    const int n = static_cast<int>(s1.size());
    const int m = static_cast<int>(s2.size());
    int match_distance = std::max(n, m) / 2 - 1;
    if (match_distance < 0) match_distance = 0;

    std::vector<bool> s1_matches(n, false);
    std::vector<bool> s2_matches(m, false);

    int matches = 0;
    for (int i = 0; i < n; ++i) {
        int start = std::max(0, i - match_distance);
        int end   = std::min(m, i + match_distance + 1);
        for (int j = start; j < end; ++j) {
            if (s2_matches[j]) continue;
            if (s1[i] != s2[j]) continue;
            s1_matches[i] = true;
            s2_matches[j] = true;
            ++matches;
            break;
        }
    }
    if (matches == 0) return 0.0;

    double transpositions = 0.0;
    int k = 0;
    for (int i = 0; i < n; ++i) {
        if (!s1_matches[i]) continue;
        while (!s2_matches[k]) ++k;
        if (s1[i] != s2[k]) transpositions += 0.5;
        ++k;
    }

    double mm = static_cast<double>(matches);
    return (mm / n + mm / m + (mm - transpositions) / mm) / 3.0;
}

}  // namespace

std::size_t utf8_length(const std::string& s) {
    std::size_t count = 0;
    std::size_t i = 0;
    while (i < s.size()) {
        int len = utf8_char_len(static_cast<unsigned char>(s[i]));
        if (len == 0) { ++i; continue; }
        i += len;
        ++count;
    }
    return count;
}

std::string normalize(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    bool last_space = true;  // чтобы срезать ведущие пробелы

    std::size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c < 0x80) {
            if (std::isalnum(c)) {
                out.push_back(static_cast<char>(std::tolower(c)));
                last_space = false;
            } else if (std::isspace(c)) {
                if (!last_space) {
                    out.push_back(' ');
                    last_space = true;
                }
            }
            // ASCII-пунктуация — пропускаем
            ++i;
        } else {
            // Многобайтовый символ — копируем целиком
            int len = utf8_char_len(c);
            if (len == 0 || i + len > s.size()) { ++i; continue; }
            out.append(s, i, len);
            last_space = false;
            i += len;
        }
    }
    if (!out.empty() && out.back() == ' ') out.pop_back();
    return out;
}

int levenshtein_distance(const std::string& a, const std::string& b) {
    return levenshtein_distance_cp(utf8_decode(a), utf8_decode(b));
}

double levenshtein_score(const std::string& a, const std::string& b) {
    if (a == b) return 1.0;
    int dist = levenshtein_distance(a, b);
    std::size_t max_len = std::max(utf8_length(a), utf8_length(b));
    if (max_len == 0) return 1.0;
    double s = 1.0 - static_cast<double>(dist) / static_cast<double>(max_len);
    return std::clamp(s, 0.0, 1.0);
}

double jaro_winkler(const std::string& a, const std::string& b) {
    auto va = utf8_decode(a);
    auto vb = utf8_decode(b);

    double j = jaro_cp(va, vb);
    if (j == 0.0) return 0.0;

    // Winkler boost: общий префикс до 4 символов
    std::size_t max_prefix = std::min({va.size(), vb.size(), std::size_t{4}});
    std::size_t prefix = 0;
    for (std::size_t i = 0; i < max_prefix; ++i) {
        if (va[i] == vb[i]) ++prefix;
        else break;
    }
    return j + 0.1 * static_cast<double>(prefix) * (1.0 - j);
}

double score(const std::string& a, const std::string& b) {
    std::string na = normalize(a);
    std::string nb = normalize(b);

    if (na.empty() && nb.empty()) return 1.0;
    if (na.empty() || nb.empty()) return 0.0;
    if (na == nb) return 1.0;

    double lev = levenshtein_score(na, nb);
    double jw  = jaro_winkler(na, nb);
    return std::max(lev, jw);
}

}  // namespace llp::similarity