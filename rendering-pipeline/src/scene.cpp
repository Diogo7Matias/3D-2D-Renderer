#include "scene.h"
#include <memory>
#include "math/mat4.h"

void Scene::add(Mesh mesh) {
    addMaterial(mesh.material());
    addObject(mesh);
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
