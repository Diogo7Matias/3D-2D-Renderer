#pragma once

#include <cstdint>
#include "../math/vec3.h"

class Color {
    uint32_t _value;

public:
    Color() { _value = 0xffffff; }
    Color(uint32_t value) : _value(value) {}

    static Color fromVec3(const Vec3& v) {
        uint8_t r = (uint8_t)(v.x * 255.0f);
        uint8_t g = (uint8_t)(v.y * 255.0f);
        uint8_t b = (uint8_t)(v.z * 255.0f);
        return Color(0xFF000000 | (r << 16) | (g << 8) | b);
    }

    uint32_t value() const { return _value; }
    void setValue(uint32_t value) { _value = value; }
    void setValue(Vec3 v) {
        uint8_t r = (uint8_t)(v.x * 255.0f);
        uint8_t g = (uint8_t)(v.y * 255.0f);
        uint8_t b = (uint8_t)(v.z * 255.0f);
        _value = (0xFF000000 | (r << 16) | (g << 8) | b);
    }

    // normalized to [0, 1]
    float red() const { return ((_value >> 16) & 0xFF) / 255.0f; }
    float green() const { return ((_value >> 8) & 0xFF) / 255.0f; }
    float blue() const { return ((_value >> 0) & 0xFF) / 255.0f; }
    
    Vec3 asVec3() const {
        return Vec3(red(), green(), blue());
    }

    Color toGreyscale() const {
        Vec3 grey = Vec3(0.299 * red()) + (0.587 * green()) + (0.114 * blue());
        return Color::fromVec3(grey);
    }

    ///
    ////// * Operator Overloads
    ///
    
    Color operator + (const Color &other) const {
        return Color::fromVec3(
            Vec3(
                std::min(red() + other.red(), 1.0f),
                std::min(green() + other.green(), 1.0f),
                std::min(blue() + other.blue(), 1.0f)
            )
        );
    }

    Color& operator += (const Color &other) { 
        setValue(
            Vec3(
                std::min(red() + other.red(), 1.0f),
                std::min(green() + other.green(), 1.0f),
                std::min(blue() + other.blue(), 1.0f)
            )
        );
        return *this;
    }

    Color operator * (const float t) const { return Color::fromVec3(Vec3(red() * t, green() * t, blue() * t)); }

    Color operator / (const float t) const { return Color::fromVec3(Vec3(red() / t, green() / t, blue() / t)); }
};