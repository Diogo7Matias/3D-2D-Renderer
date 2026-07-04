#pragma once

#include "geometry.h"
#include "material.h"

class Mesh {
    Geometry::Primitive _geometry;
    Material _material;

public:
    Mesh(Geometry::Primitive geometry, Material material) : _geometry(geometry), _material(material) {}

    Geometry::Primitive &geometry() { return _geometry; }

    Material &material() { return _material; }
};