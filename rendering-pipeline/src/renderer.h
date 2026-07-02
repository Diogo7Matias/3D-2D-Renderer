#pragma once

#include "scene.h"
#include "camera.h"
#include "window.h"
#include "color.h"

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

public:
    Renderer(Window* window) 
        : _window(window),
        _fragmentBuffer(_window->getWidth(), _window->getHeight()),
        _zbuffer(_window->getWidth(), _window->getHeight()) 
    {
        _fragmentBuffer.clear();
        _zbuffer.clear(std::numeric_limits<float>::infinity());
    }

public:
    void render(const Scene &scene, const Camera &camera);

    Buffer<Fragment> fragmentBuffer() const { return _fragmentBuffer; }
    Buffer<float> zbuffer() const { return _zbuffer; }

private:
    void setFragmentColor(const Vec3 &v, Color &color);
    bool isBackFace(const Triangle &t, const std::vector<Vertex> &vertices);
    Vec3 clipAndProject(const Vec3& pos, const Mat4& projection);
    void clipping(Vec3 &v);
    void line(int x0, int y0, int x1, int y1);
    void triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, Color color);
};