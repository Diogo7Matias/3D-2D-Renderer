#pragma once

#include "math/vec3.h"
#include "utils/color.h"

struct Vertex {
    Vec3 position;
    Vec3 normal;
    Color color;
    int materialIndex = -1; // -1 means unassigned

    Vertex(Vec3 position, Vec3 normal, int materialIndex) : position(position), normal(normal), materialIndex(materialIndex) {}
    Vertex(Vec3 position, Vec3 normal) : position(position), normal(normal) {}
};

struct Triangle {
    int v0, v1, v2; // indices into the vertex array

    Triangle(int v0, int v1, int v2) : v0(v0), v1(v1), v2(v2) {}
};