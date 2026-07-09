#pragma once

#include <cmath>
#include <iostream>

struct Vec4;

struct Vec3 {
    
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    Vec3(float v) : x(v), y(v), z(v) {}

    Vec4 toVec4(float w = 1.0f) const;

    ///
    ////// * Operator Overloads
    ///

    Vec3 operator+(const Vec3& other) const { return {x + other.x, y + other.y, z + other.z}; }
    Vec3 operator-(const Vec3& other) const { return {x - other.x, y - other.y, z - other.z}; }
    Vec3 operator*(float t) const { return {x * t, y * t, z * t}; }
    Vec3 operator/(float t) const { return {x / t, y / t, z / t}; }
    Vec3 operator- () const { return Vec3(-x, -y, -z); }
    Vec3& operator+=(const Vec3& other) { x += other.x, y += other.y, z += other.z; return *this; }

    ///
    ////// * Methods
    ///

    float dot(const Vec3& other) const { return x * other.x + y * other.y + z * other.z; }
    
    Vec3 cross(const Vec3& other) const {
        return {
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        };
    }

    Vec3 normalize() const {
        float length = std::sqrt(x * x + y * y + z * z);
        if (length == 0) return {0, 0, 0};
        return {x / length, y / length, z / length};
    }

    float length() const {
        return std::sqrt(x *x + y * y + z * z);
    }

    void print() const {
        std::cout << "(" << x << ", " << y << ", " << z << ")";
    }
};
