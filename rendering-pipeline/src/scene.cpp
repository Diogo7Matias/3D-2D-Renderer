#include "scene.h"
#include <memory>

void Scene::add(Mesh mesh) {
    int materialIndex = _materials.size();
    addMaterial(mesh.material());

    const int vertexOffset = static_cast<int>(_vertices.size());

    for (Vertex v : mesh.geometry().getVertices()) {
        v.materialIndex = materialIndex;
        addVertex(v);
    }

    for (const auto& edge : mesh.geometry().getEdges()) {
        addEdge(edge.first + vertexOffset, edge.second + vertexOffset);
    }

    for (const auto& tri : mesh.geometry().getTriangles()) {
        addTriangle(Triangle(
            tri.v0 + vertexOffset,
            tri.v1 + vertexOffset,
            tri.v2 + vertexOffset
        ));
    }
}

void Scene::add(std::unique_ptr<Camera> camera) {
    _cameras.push_back(std::move(camera));
}

void Scene::add(std::unique_ptr<Light> light) {
    _lights.push_back(std::move(light));
}

std::vector<Light*> Scene::getLights() const {
    std::vector<Light*> lights;
    for (const auto& light : _lights) {
        lights.push_back(light.get());
    }
    return lights;
}

Camera& Scene::getCamera(int index) const {
    return *_cameras[index];
}
