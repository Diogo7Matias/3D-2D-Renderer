#include "geometry.h"

namespace Geometry {

    Cube::Cube(float size) : _size(size) {
        float h = size / 2.0f;

        // +X face
        buildFace(
            Vec3( h, -h, -h),
            Vec3( h,  h, -h),
            Vec3( h,  h,  h),
            Vec3( h, -h,  h),
            Vec3(1, 0, 0)
        );
        // -X face
        buildFace(
            Vec3(-h, -h,  h),
            Vec3(-h,  h,  h),
            Vec3(-h,  h, -h),
            Vec3(-h, -h, -h),
            Vec3(-1, 0, 0)
        );
        // +Y face
        buildFace(
            Vec3(-h,  h, -h),
            Vec3(-h,  h,  h),
            Vec3( h,  h,  h),
            Vec3( h,  h, -h),
            Vec3(0, 1, 0)
        );
        // -Y face
        buildFace(
            Vec3(-h, -h,  h),
            Vec3(-h, -h, -h),
            Vec3( h, -h, -h),
            Vec3( h, -h,  h),
            Vec3(0, -1, 0)
        );
        // +Z face
        buildFace(
            Vec3(-h, -h,  h),
            Vec3( h, -h,  h),
            Vec3( h,  h,  h),
            Vec3(-h,  h,  h),
            Vec3(0, 0, 1)
        );
        // -Z face
        buildFace(
            Vec3( h, -h, -h),
            Vec3(-h, -h, -h),
            Vec3(-h,  h, -h),
            Vec3( h,  h, -h),
            Vec3(0, 0, -1)
        );
    }
        
    void Cube::buildFace(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal) {
        int base = _vertices.size();
        _vertices.push_back(Vertex(a, normal));
        _vertices.push_back(Vertex(b, normal));
        _vertices.push_back(Vertex(c, normal));
        _vertices.push_back(Vertex(d, normal));

        _edges.push_back({base + 0, base + 1});
        _edges.push_back({base + 1, base + 2});
        _edges.push_back({base + 2, base + 3});
        _edges.push_back({base + 3, base + 0});

        _triangles.push_back(Triangle(base + 0, base + 1, base + 2));
        _triangles.push_back(Triangle(base + 0, base + 2, base + 3));
    }

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    Sphere::Sphere(float radius, size_t segments) : _radius(radius), _segments(segments) {
        // Vertices
        for (size_t i = 0; i <= _segments; ++i) {
            float phi = (float)i / (float)_segments * M_PI;
            for (size_t j = 0; j <= _segments; ++j) {
                float theta = (float)j / (float)_segments * 2 * M_PI;
                Vec3 position = Vec3(
                    _radius * std::sin(phi) * std::cos(theta),
                    _radius * std::cos(phi),
                    _radius * std::sin(phi) * std::sin(theta)
                );
                Vec3 normal = (position).normalize();
                _vertices.push_back(Vertex(position, normal));
            }
        }
        
        // Edges & Triangles
        for (size_t i = 0; i < _segments; ++i) {
            for (size_t j = 0; j < _segments; ++j) {
                int current = i * (_segments + 1) + j;
                int next_col = i * (_segments + 1) + j + 1;
                int next_row = (i + 1) * (_segments + 1) + j;
                int next_both = (i + 1) * (_segments + 1) + j + 1;

                _edges.push_back({current, next_col});
                _edges.push_back({current, next_row});
                _triangles.push_back({current, next_row, next_col});
                _triangles.push_back({next_col, next_row, next_both});
            }
        }
    }

    Sphere::Sphere(float radius) : Sphere(radius, 16) {}

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    OBJModel::OBJModel(const std::vector<Vertex>& vertices, const std::vector<Triangle>& triangles) {
        _vertices = vertices;
        _triangles = triangles;
        
        // Generate edges from triangles
        for (const auto& tri : triangles) {
            _edges.push_back({tri.v0, tri.v1});
            _edges.push_back({tri.v1, tri.v2});
            _edges.push_back({tri.v2, tri.v0});
        }
    }
}