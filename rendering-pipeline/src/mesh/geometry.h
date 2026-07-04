#pragma once

#include "../math/vec3.h"
#include "../vertex.h"
#include <vector>

namespace Geometry {

    //
    ////// Base class for primitives
    //
    class Primitive {
    protected:
        std::vector<Vertex> _vertices;
        std::vector<std::pair<int,int>> _edges;
        std::vector<Triangle> _triangles;

    public:
        virtual ~Primitive() = default;
    
        const std::vector<Vertex> &getVertices() const { return _vertices; }
        const std::vector<std::pair<int,int>> &getEdges() const { return _edges; }
        const std::vector<Triangle>& getTriangles() const { return _triangles; }
    };

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    class Cube : public Primitive {
        Vec3 _position;
        float _size;

    public:
        Cube(const Vec3 &position, float size);
        
        const Vec3 &position() const { return _position; }

        float size() const { return _size; }

    private:
        void buildFace(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Vec3 normal);
    };

    /////////////////////////////////////////////////////////////////////////////
    /////////////////////////////////////////////////////////////////////////////

    class Sphere : public Primitive {
        Vec3 _center;
        float _radius;
        size_t _segments;

    public:
        Sphere(const Vec3 &center, float radius, size_t segments);
        Sphere(const Vec3 &center, float radius);

        const Vec3 &center() const { return _center; }
        
        float radius() const { return _radius; }
    };
}