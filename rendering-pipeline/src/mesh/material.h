#pragma once

#include "../utils/color.h"

class Material {
    Color _color;

    // Reflection Coefficients
    float _ambient_k;
    float _diffusion_k;
    float _specular_k;

    float _shininess;

public:
    Material(Color color, float ambient_k, float diffusion_k, float specular_k, float shininess) 
        : _color(color), _ambient_k(ambient_k), _diffusion_k(diffusion_k), _specular_k(specular_k), _shininess(shininess) {}

    Material(Color color, float diffusion_k, float specular_k, float shininess) 
        : _color(color), _diffusion_k(diffusion_k), _specular_k(specular_k), _shininess(shininess) {
            _ambient_k = 1;
        }

    virtual ~Material() = default;

    Color getColor() const { return _color; }
    float ambientK() const { return _ambient_k; }
    float diffusionK() const { return _diffusion_k; }
    float specularK() const { return _specular_k; }
    float shininess() const { return _shininess; }
};

// --------------------------------------------------------------------------

class DiffuseMaterial : public Material {

public:
    DiffuseMaterial(Color color, float ambient_k, float diffusion_k) : Material(color, ambient_k, diffusion_k, 0, 0) {}
    DiffuseMaterial(Color color, float diffusion_k) : Material(color, diffusion_k, 0, 0) {}
};