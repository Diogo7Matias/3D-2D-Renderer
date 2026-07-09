#pragma once

#include "../utils/color.h"

enum ShadingModel {
    FLAT,
    GOURAUD,
    PHONG
};

class Material {
    Color _color;
    ShadingModel _shading = FLAT;

    // Reflection Coefficients
    float _ambient_k;
    float _diffusion_k;
    float _specular_k;

    float _shininess;

public:
    Material(Color color, float ambient_k, float diffusion_k, float specular_k, float shininess) 
        : _color(color), _ambient_k(ambient_k), _diffusion_k(diffusion_k), _specular_k(specular_k), _shininess(shininess) {}

    virtual ~Material() = default;

    Color getColor() const { return _color; }
    ShadingModel getShading() const { return _shading; }
    void setShading(ShadingModel shading) { _shading = shading; }
    float ambientK() const { return _ambient_k; }
    float diffusionK() const { return _diffusion_k; }
    float specularK() const { return _specular_k; }
    float shininess() const { return _shininess; }
};

// --------------------------------------------------------------------------
// Diffuse surface with a very subtle sheen
// Examples: painted walls, fabric, rough plastic
class DiffuseMaterial : public Material {
public:
    DiffuseMaterial(Color color, float diffusion_k = 0.8f, float specular_k = 0.1f)
        : Material(color, 0.1f, diffusion_k, specular_k, 8.0f) {}
};

// --------------------------------------------------------------------------
// Perfectly matte surface — no specular highlight at all
// Examples: chalk, unfinished wood, clay
class MatteMaterial : public Material {
public:
    MatteMaterial(Color color, float diffusion_k = 1.0f)
        : Material(color, 0.1f, diffusion_k, 0.0f, 0.0f) {}
};

// --------------------------------------------------------------------------
// Plastic — strong diffuse with a sharp specular highlight
// The specular color is white regardless of the material color
// Examples: toys, hard plastic surfaces
class PlasticMaterial : public Material {
public:
    PlasticMaterial(Color color, float shininess = 32.0f)
        : Material(color, 0.1f, 0.7f, 0.5f, shininess) {}
};

// --------------------------------------------------------------------------
// Metal — low diffuse, high specular, specular color matches material color
// Examples: gold, copper, aluminum
class MetalMaterial : public Material {
public:
    MetalMaterial(Color color, float shininess = 64.0f)
        : Material(color, 0.1f, 0.3f, 0.9f, shininess) {}
};

// --------------------------------------------------------------------------
// Glass/mirror-like surface — very low diffuse, very high specular
// Examples: glass, water surface, polished stone
class GlossyMaterial : public Material {
public:
    GlossyMaterial(Color color, float shininess = 128.0f)
        : Material(color, 0.05f, 0.1f, 1.0f, shininess) {}
};