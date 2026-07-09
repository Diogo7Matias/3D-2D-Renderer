#pragma once

#include "utils/color.h"
#include "mesh/material.h"

class Light {
    
protected:
    Color _color;
    float _intensity;

public:
    Light(Color color, float intensity) : _color(color), _intensity(intensity) {}
    
    Light(Color color) : _color(color) {
        _intensity = 1.0f;
    }

    Light() {
        _color = Color(0x000000);
        _intensity = 1.0f;
    }

    virtual ~Light() = default;

    virtual Color compute(const Vec3 &vPos, const Vec3 &normal, const Material &material) const = 0;
};

// --------------------------------------------------------------------------

class AmbientLight : public Light {

public:
    AmbientLight(Color color, float intensity) : Light(color, intensity) {}
    AmbientLight(Color color) : Light(color) {}
    AmbientLight() : Light() {}

    Color compute(const Vec3 &vPos, const Vec3 &normal, const Material &material) const override {
        return _color * _intensity * material.ambientK();
    }
};

// --------------------------------------------------------------------------

class PointLight : public Light {
    Vec3 _position; // in World Coordinates
    Vec3 _viewPosition; // in Camera Coordinates

public:
    PointLight(Vec3 position, Color color, float intensity) : Light(color, intensity), _position(position) {}
    PointLight(Vec3 position, Color color) : Light(color), _position(position) {}
    PointLight(Vec3 position) : Light(), _position(position) {}

    void setViewPosition(const Mat4& view) {
        _viewPosition = (view * _position.toVec4()).toVec3();
    }

    Color compute(const Vec3 &vPos, const Vec3 &normal, const Material &material) const override {
        Vec3 L = (_viewPosition - vPos).normalize();
        Vec3 V = (-vPos).normalize();
        Vec3 H = (L + V).normalize();

        float diffuse = std::max(0.0f, normal.dot(L));
        float specular = std::max(0.0f, std::pow(normal.dot(H), material.shininess()));

        const float a = 1.0f;
        const float b = 0.045f;
        const float c = 0.0075f;
        const float d = (_viewPosition - vPos).length();
        float attenuation = 1.0f / (a + b * d + c * d * d);

        return _color * attenuation * _intensity * (material.diffusionK() * diffuse + material.specularK() * specular);
    }
};