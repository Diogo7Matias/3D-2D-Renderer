#pragma once

#include "math/vec3.h"
#include "utils/color.h"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Color color = Color(0x000000);

    Vertex(Vec3 position, Vec3 normal, Color color) : position(position), normal(normal), color(color) {}
    Vertex(Vec3 position, Vec3 normal) : position(position), normal(normal) {}
};

struct Triangle {
    int v0, v1, v2; // indices into the vertex array of the mesh the triangle is part of

    Triangle(int v0, int v1, int v2) : v0(v0), v1(v1), v2(v2) {}
};