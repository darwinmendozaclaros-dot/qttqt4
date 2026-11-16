#pragma once
#include "raylib.h"
#include <string>

namespace Util {
    inline float Rand01() { return (float)GetRandomValue(0, 10000) / 10000.0f; }
    inline float RandRange(float a, float b) { return a + (b - a) * Rand01(); }
    // Primeros n bytes de s sin partir un caracter UTF-8 a la mitad (ñ, á, ¿...): lo usa el efecto de maquina de escribir.
    inline std::string Utf8Head(const std::string& s, size_t n) {
        if (n >= s.size()) return s;
        while (n > 0 && ((unsigned char)s[n] & 0xC0) == 0x80) --n;
        return s.substr(0, n);
    }
    inline float Clamp01(float v) { return v < 0.f ? 0.f : (v > 1.f ? 1.f : v); }

    inline Color LerpColor(Color a, Color b, float t) {
        t = Clamp01(t);
        return Color{ (unsigned char)(a.r + (b.r - a.r) * t),
                      (unsigned char)(a.g + (b.g - a.g) * t),
                      (unsigned char)(a.b + (b.b - a.b) * t),
                      (unsigned char)(a.a + (b.a - a.a) * t) };
    }
}
