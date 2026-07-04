#pragma once

#include "scene.h"
#include "camera.h"
#include "window.h"
#include "utils/color.h"

struct Fragment {
    int x, y;
    Vec3 color;
};

template <typename T>
class Buffer {
    std::vector<T> _data;
    int _width;
    int _height;

public:
    Buffer(int width, int height) : _width(width), _height(height) {
        _data = std::vector<T>(width * height);
    }

    const T* get(int x, int y) const {
        return &_data[y * _width + x];
    }

    void set(int x, int y, const T& value) {
        _data[y * _width + x] = value;
    }

    T& operator[](int index) {
        return _data[index];
    }

    void clear(const T& value = T()) {
        std::fill(_data.begin(), _data.end(), value);
    }
};

class Renderer {
    Window* _window;
    Buffer<Fragment> _fragmentBuffer;
    Buffer<float> _zbuffer;
    bool _wireframe = false;
    bool _depthShading = false;

public:
    Renderer(Window* window) 
        : _window(window),
        _fragmentBuffer(_window->getWidth(), _window->getHeight()),
        _zbuffer(_window->getWidth(), _window->getHeight()) 
    {
        _fragmentBuffer.clear();
        _zbuffer.clear(std::numeric_limits<float>::infinity());
    }

    Buffer<Fragment> fragmentBuffer() const { return _fragmentBuffer; }
    Buffer<float> zbuffer() const { return _zbuffer; }

public:
    /**
     * The main function of this renderer.
     * It renders the scene from the perspective of the given camera.
     * The result is stored in the fragment buffer.
     * @param scene The scene to render.
     * @param camera The camera to render from.
     */
    void render(const Scene &scene, const Camera &camera);

    /**
     * Sets the wireframe mode.
     * @param on true to enable wireframe mode, false to disable it.
     */
    void setWireframe(bool on) { _wireframe = on; }

    /**
     * Sets the depth shading mode.
     * @param on true to enable depth shading, false to disable it.
     */
    void setDepthShading(bool on) { _depthShading = on; }

private:
    void setFragmentColor(const Vec3 &v, Color &color);
    Color applyDepthShading(const Color &color, float z) const;
    bool isBackFace(const Triangle &t, const std::vector<Vertex> &vertices);
    Vec3 clipAndProject(const Vec3& pos, const Mat4& projection);
    void clipping(Vec3 &v);
    void line(const Vec3 &p0, const Vec3 &p1, float z0, float z1, const Color &color);
    void triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, float z1, float z2, float z3, const Color &color);
};