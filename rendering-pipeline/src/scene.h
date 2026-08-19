#pragma once

#include "math/vec3.h"
#include "mesh/mesh.h"
#include "mesh/geometry.h"
#include "camera.h"
#include "light.h"

#include <vector>
#include <memory>

class Scene {
    std::vector<Mesh> _objects;

    std::vector<Material> _materials;
    
    std::vector<std::unique_ptr<Light>> _lights;
    std::vector<std::unique_ptr<Camera>> _cameras;

public:
    Scene() {}
    
    std::vector<Mesh> objects() const { return _objects; }
    std::vector<Material> materials() const { return _materials; }

    void add(Mesh mesh);
    void add(std::unique_ptr<Camera> camera);
    void add(std::unique_ptr<Light> light);

    std::vector<Light*> getLights() const;
    
    Camera& getCamera(int index) const;
    int cameraCount() const { return _cameras.size(); }

private:
    void addObject(Mesh object) {
        _objects.push_back(object);
    }

    void addMaterial(Material material) {
        _materials.push_back(material);
    }
};
