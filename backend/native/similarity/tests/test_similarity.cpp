#include "llp/similarity/similarity.hpp"

#include <cassert>
#include <cmath>
#include <iostream>

using namespace llp::similarity;

void test_normalize_basic() {
    std::string s = normalize("  Hello, World!  ");
    std::cout << "[OK] normalize basic: '" << s << "'\n";
    assert(s == "hello world");
}

void test_normalize_unicode_preserved() {
    std::string s = normalize("  Язык,  Слово!  ");
    std::cout << "[OK] normalize unicode: '" << s << "'\n";
    assert(s == "язык слово");
}

void test_exact_match() {
    double s = score("язык", "язык");
    std::cout << "[OK] exact: " << s << "\n";
    assert(std::abs(s - 1.0) < 1e-9);
}

void test_case_insensitive() {
    double s = score("Hello", "hello");
    std::cout << "[OK] case-insensitive: " << s << "\n";
    assert(std::abs(s - 1.0) < 1e-9);
}

void test_punctuation_ignored() {
    double s = score("hello!", "hello");
    std::cout << "[OK] punctuation: " << s << "\n";
    assert(std::abs(s - 1.0) < 1e-9);
}

void test_one_edit_latin() {
    double s = score("язык", "языки");
    std::cout << "[OK] one edit latin: " << s << "\n";
    assert(s > 0.7);
}

void test_one_edit_japanese() {
    double s = score("ありがとう", "ありがと");
    std::cout << "[OK] one edit japanese: " << s << "\n";
    assert(s > 0.8);
}

void test_different_japanese() {
    double s = score("ありがとう", "こんにちは");
    std::cout << "[OK] different japanese: " << s << "\n";
    assert(s < 0.5);
}

void test_different_latin() {
    double s = score("кот", "пёс");
    std::cout << "[OK] different latin: " << s << "\n";
    assert(s < 0.5);
}

void test_empty_both() {
    double s = score("", "");
    std::cout << "[OK] empty both: " << s << "\n";
    assert(std::abs(s - 1.0) < 1e-9);
}

void test_empty_one() {
    double s = score("", "hello");
    std::cout << "[OK] empty one: " << s << "\n";
    assert(std::abs(s - 0.0) < 1e-9);
}

void test_levenshtein_distance() {
    int d = levenshtein_distance("kitten", "sitting");
    std::cout << "[OK] levenshtein(kitten, sitting) = " << d << "\n";
    assert(d == 3);
}

void test_utf8_length() {
    std::size_t len = utf8_length("ありがとう");
    std::cout << "[OK] utf8_length(ありがとう) = " << len << "\n";
    assert(len == 5);
}

void test_multiple_choice() {
    // "речь, язык" vs "язык" — должен быть средний score
    double s = score("речь, язык", "язык");
    std::cout << "[OK] partial: " << s << "\n";
    assert(s > 0.4 && s < 0.9);
}

int main() {
    test_normalize_basic();
    test_normalize_unicode_preserved();
    test_exact_match();
    test_case_insensitive();
    test_punctuation_ignored();
    test_one_edit_latin();
    test_one_edit_japanese();
    test_different_japanese();
    test_different_latin();
    test_empty_both();
    test_empty_one();
    test_levenshtein_distance();
    test_utf8_length();
    test_multiple_choice();
    std::cout << "\nAll similarity tests passed.\n";
    return 0;
}