#pragma once

#include "math/vec3.h"
#include "math/mat4.h"
#include <algorithm>

class Camera {
    // in World Coordinates
    Vec3 _position;
    Vec3 _direction;    // normalized
    Vec3 _up;           // normalized
    float _zoom = 1.0;

public:
    Camera(Vec3 position) : _position(position) {
        _direction = Vec3(0, 0, -1);
        _up = Vec3(0, 1, 0);
    }

    Camera() {
        _position = Vec3(0, 0, 0);
        _direction = Vec3(0, 0, -1);
        _up = Vec3(0, 1, 0);
    }

    virtual ~Camera() = default;

    const Vec3 &position() const { return _position; }

    const Vec3 &direction() const { return _direction; }

    const Vec3 &up() const { return _up; }

    float getZoom() const { return _zoom; }

    void zoom(float factor)  {
        _zoom *= factor;
        // TODO: adjust clamp later. Commented out for debug purposes
        // _zoom = std::clamp(_zoom, 0.5f, 10.0f);
    }

    void setPosition(Vec3 position) {
        _position = position;
    }

    void lookAt(Vec3 lookAt) {
        _direction = (lookAt - _position).normalize();
    }

    void setRoll(Vec3 up) {
        _up = up - _direction * (_direction.dot(up));
        _up = _up.normalize();
    }

    Mat4 viewMatrix() const {
        Vec3 v = up();
        Vec3 n = direction();
        Vec3 u = n.cross(v).normalize();

        Mat4 translation = Mat4::translation(-position());
        Mat4 rotation = Mat4::fromBasis(u, v, -n);
        Mat4 scale = Mat4::scale(Vec3(1, 1, -1));
        return scale * rotation * translation;
    }

    virtual Mat4 projectionMatrix() const = 0;
};

/* .............. Orthographic Camera .............. */

class OrthographicCamera : public Camera {
    float _left, _right, _top, _bottom;
    float _near, _far;

public:
    OrthographicCamera(Vec3 position, float left, float right, float top, float bottom, float near, float far)
        : Camera(position), _left(left), _right(right), _top(top), _bottom(bottom), _near(near), _far(far) {}

    OrthographicCamera(float left, float right, float top, float bottom, float near, float far)
        : Camera(), _left(left), _right(right), _top(top), _bottom(bottom), _near(near), _far(far) {}

    float near() const { return _near; }

    float far() const { return _far; }

    float left() const { return _left; }

    float right() const { return _right; }

    float top() const { return _top; }
    
    float bottom() const { return _bottom; }

    Mat4 projectionMatrix() const override {
        float left = _left / getZoom();
        float top = _top / getZoom();
        float right = _right / getZoom();
        float bottom = _bottom / getZoom();

        float tx = -(right + left) / 2;
        float ty = -(top + bottom) / 2;
        float tz = (_far + _near) / 2;
        Mat4 translation = Mat4::translation(Vec3(tx, ty, tz));
        
        float sx = 2 / (right - left);
        float sy = 2 / (top - bottom);
        float sz = -2 / (_far - _near);
        Mat4 scale = Mat4::scale(Vec3(sx, sy, sz));
        
        return scale * translation;
    }

};

/* .............. Perspective Camera .............. */

class PerspectiveCamera : public Camera {
    float _fov;
    float _aspectRatio;
    float _near, _far;

public:
    PerspectiveCamera(Vec3 position, float fov, float aspectRatio, float near, float far) 
        : Camera(position), _fov(fov), _aspectRatio(aspectRatio), _near(near), _far(far) {}
    
    PerspectiveCamera(float fov, float aspectRatio, float near, float far)
        : Camera(), _fov(fov), _aspectRatio(aspectRatio), _near(near), _far(far) {}
    
    float fov() const { return _fov; }

    void setFov(float fov) { _fov = fov; }

    float aspectRatio() const { return _aspectRatio; }

    void setAspect(float aspectRatio) { _aspectRatio = aspectRatio; }
    
    float near() const { return _near; }

    float far() const { return _far; }

    Mat4 projectionMatrix() const override {
        float fov_rad = _fov * M_PI / 180.0f;
        float f = getZoom() / std::tan(fov_rad / 2.0f);
        Vec4 u1 = Vec4(f / _aspectRatio, 0, 0, 0);
        Vec4 u2 = Vec4(0, f, 0, 0);
        Vec4 u3 = Vec4(0, 0, (_far + _near) / (_far - _near), (2 * _far * _near) / (_near - _far));
        Vec4 u4 = Vec4(0, 0, 1, 0);

        return Mat4::fromRows(u1, u2, u3, u4);
    }
};