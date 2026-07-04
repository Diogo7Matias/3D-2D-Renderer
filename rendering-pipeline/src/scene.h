#pragma once

#include "math/vec3.h"
#include "mesh/mesh.h"
#include "mesh/geometry.h"
#include "camera.h"
#include "light.h"

#include <vector>
#include <memory>

class Scene {
    std::vector<Vertex> _vertices;
    std::vector<std::pair<int,int>> _edges;
    std::vector<Triangle> _triangles;

    std::vector<Material> _materials;
    
    std::vector<std::unique_ptr<Light>> _lights;
    std::vector<std::unique_ptr<Camera>> _cameras;

public:
    Scene() {}
    
    std::vector<Vertex> vertices() const { return _vertices; }
    std::vector<std::pair<int,int>> edges() const { return _edges; }
    std::vector<Triangle> triangles() const { return _triangles; }
    std::vector<Material> materials() const { return _materials; }

    void add(Mesh mesh);
    void add(std::unique_ptr<Camera> camera);
    void add(std::unique_ptr<Light> light);

    std::vector<Light*> getLights() const;
    
    Camera& getCamera(int index) const;
    int cameraCount() const { return _cameras.size(); }

private:
    void addVertex(Vertex vertex) {
        _vertices.push_back(vertex);
    }

    void addMaterial(Material material) {
        _materials.push_back(material);
    }

    void addEdge(int index1, int index2) {
        _edges.push_back(std::make_pair(index1, index2));
    }

    void addTriangle(Triangle triangle) {
        _triangles.push_back(triangle);
    }
};
