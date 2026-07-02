#pragma once

#include "math/vec3.h"
#include "vertex.h"
#include <vector>

namespace Geometry {

    //
    ////// Base class for primitives
    //
    class Primitive {
        std::vector<Vertex> _vertices;
        std::vector<std::pair<int,int>> _edges;
        std::vector<Triangle> _triangles;

    public:
        virtual ~Primitive() = default;
    
        const std::vector<Vertex> &getVertices() const { return _vertices; }
        void setVertices(const std::vector<Vertex> &verts) { _vertices = verts; }
        const std::vector<std::pair<int,int>> &getEdges() const { return _edges; }
        void setEdges(const std::vector<std::pair<int,int>> &edges) { _edges = edges; }
        const std::vector<Triangle>& getTriangles() const { return _triangles; }
        void setTriangles(const std::vector<Triangle>& tris) { _triangles = tris; }
    };

    // --------------------------------------------------------------------------

    class Cube : public Primitive {
        Vec3 _position;
        float _size;

    public:
        Cube(const Vec3 &position, float size) : _position(position), _size(size) {
            std::vector<Vertex> vertices;
            std::vector<std::pair<int,int>> edges;
            std::vector<Triangle> triangles;
            float h = size / 2.0f;

            // +X face
            buildFace(
                position + Vec3( h, -h, -h),
                position + Vec3( h,  h, -h),
                position + Vec3( h,  h,  h),
                position + Vec3( h, -h,  h),
                Vec3(1, 0, 0), vertices, edges, triangles
            );
            // -X face
            buildFace(
                position + Vec3(-h, -h,  h),
                position + Vec3(-h,  h,  h),
                position + Vec3(-h,  h, -h),
                position + Vec3(-h, -h, -h),
                Vec3(-1, 0, 0), vertices, edges, triangles
            );
            // +Y face
            buildFace(
                position + Vec3(-h,  h, -h),
                position + Vec3(-h,  h,  h),
                position + Vec3( h,  h,  h),
                position + Vec3( h,  h, -h),
                Vec3(0, 1, 0), vertices, edges, triangles
            );
            // -Y face
            buildFace(
                position + Vec3(-h, -h,  h),
                position + Vec3(-h, -h, -h),
                position + Vec3( h, -h, -h),
                position + Vec3( h, -h,  h),
                Vec3(0, -1, 0), vertices, edges, triangles
            );
            // +Z face
            buildFace(
                position + Vec3(-h, -h,  h),
                position + Vec3( h, -h,  h),
                position + Vec3( h,  h,  h),
                position + Vec3(-h,  h,  h),
                Vec3(0, 0, 1), vertices, edges, triangles
            );
            // -Z face
            buildFace(
                position + Vec3( h, -h, -h),
                position + Vec3(-h, -h, -h),
                position + Vec3(-h,  h, -h),
                position + Vec3( h,  h, -h),
                Vec3(0, 0, -1), vertices, edges, triangles
            );

            setVertices(vertices);
            setEdges(edges);
            setTriangles(triangles);
        }
        
        const Vec3 &position() const { return _position; }

        float size() const { return _size; }

    private:
        void buildFace(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal,
                    std::vector<Vertex>& vertices,
                    std::vector<std::pair<int,int>>& edges,
                    std::vector<Triangle>& triangles) {
            int base = vertices.size();
            vertices.push_back(Vertex(a, normal));
            vertices.push_back(Vertex(b, normal));
            vertices.push_back(Vertex(c, normal));
            vertices.push_back(Vertex(d, normal));

            edges.push_back({base + 0, base + 1});
            edges.push_back({base + 1, base + 2});
            edges.push_back({base + 2, base + 3});
            edges.push_back({base + 3, base + 0});

            triangles.push_back(Triangle(base + 0, base + 1, base + 2));
            triangles.push_back(Triangle(base + 0, base + 2, base + 3));
        }
    };

    // --------------------------------------------------------------------------

    class Sphere : public Primitive {
        Vec3 _center;
        float _radius;
        size_t _segments;

    public:
        Sphere(const Vec3 &center, float radius, size_t segments) : _center(center), _radius(radius), _segments(segments) {
            std::vector<Vertex> vertices;
            std::vector<std::pair<int,int>> edges;
            std::vector<Triangle> triangles;

            // Vertices
            for (size_t i = 0; i <= _segments; ++i) {
                float phi = (float)i / (float)_segments * M_PI;
                for (size_t j = 0; j <= _segments; ++j) {
                    float theta = (float)j / (float)_segments * 2 * M_PI;
                    Vec3 position = Vec3(
                        _center.x + _radius * std::sin(phi) * std::cos(theta),
                        _center.y + _radius * std::cos(phi),
                        _center.z + _radius * std::sin(phi) * std::sin(theta)
                    );
                    Vec3 normal = (position - _center).normalize();
                    vertices.push_back(Vertex(position, normal));
                }
            }
            
            // Edges & Triangles
            for (size_t i = 0; i < _segments; ++i) {
                for (size_t j = 0; j < _segments; ++j) {
                    int current = i * (_segments + 1) + j;
                    int next_col = i * (_segments + 1) + j + 1;
                    int next_row = (i + 1) * (_segments + 1) + j;
                    int next_both = (i + 1) * (_segments + 1) + j + 1;

                    edges.push_back({current, next_col});
                    edges.push_back({current, next_row});
                    triangles.push_back({current, next_row, next_col});
                    triangles.push_back({next_col, next_row, next_both});
                }
            }

            setVertices(vertices);
            setEdges(edges);
            setTriangles(triangles);
        }

        Sphere(const Vec3 &center, float radius) : Sphere(center, radius, 16) {}

        const Vec3 &center() const { return _center; }
        
        float radius() const { return _radius; }
    };
}