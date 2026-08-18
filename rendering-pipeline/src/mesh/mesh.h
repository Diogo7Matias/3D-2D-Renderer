#pragma once

#include "geometry.h"
#include "material.h"

class Mesh {
    Geometry::Primitive _geometry;
    Material _material;

    Vec3 _position = Vec3(0,0,0);
    Vec3 _rotation = Vec3(0,0,0); // radians
    Vec3 _scale = Vec3(1,1,1);

public:
    Mesh(Geometry::Primitive geometry, Material material) : _geometry(geometry), _material(material) {}

    Geometry::Primitive &geometry() { return _geometry; }

    Material &material() { return _material; }
    
    const Vec3 &position() const { return _position; }
    const Vec3 &rotation() const { return _rotation; }
    const Vec3 &scale() const { return _scale; }

    void setPosition(const Vec3 &p) { _position = p; }
    void setRotation(const Vec3 &radians) { _rotation = radians; }
    void setRotationDegrees(const Vec3 &degrees) { _rotation = Vec3(degrees.x * M_PI / 180.0f, degrees.y * M_PI / 180.0f, degrees.z * M_PI / 180.0f); }
    void setScale(const Vec3 &s) { _scale = s; }
};