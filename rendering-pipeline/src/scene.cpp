#include "scene.h"
#include <memory>
#include "math/mat4.h"

void Scene::add(Mesh mesh) {
    int materialIndex = _materials.size();
    addMaterial(mesh.material());

    const int vertexOffset = static_cast<int>(_vertices.size());

    // model transform
    Mat4 translate = Mat4::translation(mesh.position());
    Mat4 rotate = Mat4::rotation(mesh.rotation().x, mesh.rotation().y, mesh.rotation().z);
    Mat4 scale = Mat4::scale(mesh.scale());
    Mat4 model = translate * rotate * scale;

    // normal transformation (no translation)
    Mat4 normalTransform = (rotate * scale).inverseTranspose();

    for (Vertex v : mesh.geometry().getVertices()) {
        // transform position
        Vec4 p4 = model * v.position.toVec4();
        v.position = p4.toVec3();

        // transform normal (w = 0)
        Vec4 n4 = normalTransform * Vec4(v.normal.x, v.normal.y, v.normal.z, 0.0f);
        v.normal = n4.toVec3().normalize();

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
