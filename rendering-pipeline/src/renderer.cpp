#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

#include "renderer.h"
#include "math/vec4.h"
#include "math/mat4.h"
#include "color.h"

void Renderer::render(const Scene &scene, const Camera &camera) {
    std::vector<Vertex> vertices = scene.vertices();
    std::vector<Material> materials = scene.materials();
    std::vector<Light*> lights = scene.getLights();
    
    _fragmentBuffer.clear();
    _zbuffer.clear(std::numeric_limits<float>::infinity());

    Mat4 view = camera.viewMatrix();
    Mat4 projection = camera.projectionMatrix();
    Mat4 viewport = _window->viewportMatrix();
    Mat4 normalMatrix = view.inverseTranspose();

    // transform lights and vertices to view space (camera coordinates)
    for (Light* light : lights) {
        if (PointLight* pl = dynamic_cast<PointLight*>(light)) {
            pl->setViewPosition(view);
        }
    }
    for (Vertex &v : vertices) {
        v.position = (view * v.position.toVec4()).toVec3();
        Vec4 n = normalMatrix * Vec4(v.normal.x, v.normal.y, v.normal.z, 0.0f);
        v.normal = Vec3(n.x, n.y, n.z).normalize();
    }

    for (const Triangle& t : scene.triangles()) {
        Vertex& v0 = vertices[t.v0];
        Vertex& v1 = vertices[t.v1];
        Vertex& v2 = vertices[t.v2];

        if (isBackFace(t, vertices)) continue;

        // assuming flat shading only for now
        Vec3 centroid = (v0.position + v1.position + v2.position) / 3.0f;
        Vec3 normal = (v0.normal + v1.normal + v2.normal).normalize();

        Color color = materials[v0.materialIndex].getColor();
        for (Light* light : lights) {
            color += light->compute(centroid, normal, materials[v0.materialIndex]);
        }

        Vec3 p0 = (viewport * clipAndProject(v0.position, projection).toVec4()).toVec3();
        Vec3 p1 = (viewport * clipAndProject(v1.position, projection).toVec4()).toVec3();
        Vec3 p2 = (viewport * clipAndProject(v2.position, projection).toVec4()).toVec3();

        triangle(p0, p1, p2, color);
    }

    // wireframe
    // for (const auto& edge : scene.edges()) {
    //     int x0 = (int)vertices[edge.first].position.x;
    //     int y0 = (int)vertices[edge.first].position.y;
    //     int x1 = (int)vertices[edge.second].position.x;
    //     int y1 = (int)vertices[edge.second].position.y;
    //     line(x0, y0, x1, y1);
    // }
}

Vec3 Renderer::clipAndProject(const Vec3& pos, const Mat4& projection) {
    Vec3 p = (projection * pos.toVec4()).toVec3();
    clipping(p);
    return p;
}

bool Renderer::isBackFace(const Triangle &t, const std::vector<Vertex> &vertices) {
    const Vertex& v0 = vertices[t.v0];
    const Vertex& v1 = vertices[t.v1];
    const Vertex& v2 = vertices[t.v2];
    Vec3 normal = (v0.normal + v1.normal + v2.normal).normalize();
    return normal.z > 0;
}

// Cohen-Sutherland clipping algorithm
void Renderer::clipping(Vec3 &v) {
    // Define the clipping boundaries
    const float xMin = -1.0f, xMax = 1.0f;
    const float yMin = -1.0f, yMax = 1.0f;
    const float zMin = -1.0f, zMax = 1.0f;

    // Compute the outcode for the point
    int outcode = 0;
    if (v.x < xMin) outcode |= 1; // left
    else if (v.x > xMax) outcode |= 2; // right

    if (v.y < yMin) outcode |= 4; // bottom
    else if (v.y > yMax) outcode |= 8; // top
    
    if (v.z < zMin) outcode |= 16; // near
    else if (v.z > zMax) outcode |= 32; // far

    // If the point is outside the clipping volume, set it to the nearest boundary
    if (outcode != 0) {
        if (outcode & 1) v.x = xMin; // left
        else if (outcode & 2) v.x = xMax; // right

        if (outcode & 4) v.y = yMin; // bottom
        else if (outcode & 8) v.y = yMax; // top

        if (outcode & 16) v.z = zMin; // near
        else if (outcode & 32) v.z = zMax; // far
    }
}

// Writes vertex to fragment buffer
void Renderer::setFragmentColor(const Vec3 &v, Color &color) {
    int x = (int)v.x;
    int y = (int)v.y;
    
    if (x >= 0 && x < _window->getWidth() && y >= 0 && y < _window->getHeight()) {
        _fragmentBuffer.set(x, y, {x, y, color.asVec3()});
    }
}

// this function implements the Bresenham's line drawing algorithm
void Renderer::line(int x0, int y0, int x1, int y1){
    // parameterization of the segment [A, B] where A = (x0, y0), B = (x1, y1):
    //      x(t) = x0 + t * (x1 - x0)
    //      y(t) = y0 + t * (y1 - y0)
    // t is a parameter between 0 and 1.

    if (x0 == x1 && y0 == y1) {
        _fragmentBuffer.set(x0, y0, {x0, y0, Vec3(1, 1, 1)}); // white pixel
        return;
    }

    // if the line is steep (more "vertical" than "horizontal"), transpose the coordinates
    bool steep = std::abs(x0 - x1) < std::abs(y0 - y1);
    if (steep){
        std::swap(x0, y0);
        std::swap(x1, y1);
    }
    // if the starting point is to the right of the other end of the segment, swap endpoints
    if (x0 > x1){
        std::swap(x0, x1);
        std::swap(y0, y1);
    }

    // main component of the algorithm where we iterate over x to draw the line
    for (int x = x0; x <= x1; x++){
        float t = (x - x0) / (float)(x1 - x0);
        int y = std::round(y0 * (1. - t) + y1 * t);

        // check if we had previously transposed the coordinates and de-transpose if so
        if (steep){
            _fragmentBuffer.set(y, x, {y, x, Vec3(1, 1, 1)}); // white pixel
        } else {
            _fragmentBuffer.set(x, y, {x, y, Vec3(1, 1, 1)}); // white pixel
        }
    }
}

double signed_triangle_area(float x0, float y0, float x1, float y1, float x2, float y2){
    return ((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) / 2.0;
}

void Renderer::triangle(const Vec3 &p1, const Vec3 &p2, const Vec3 &p3, Color color){
    int xmin = std::min(std::min(p1.x, p2.x), p3.x);
    int ymin = std::min(std::min(p1.y, p2.y), p3.y);
    int xmax = std::max(std::max(p1.x, p2.x), p3.x);
    int ymax = std::max(std::max(p1.y, p2.y), p3.y);
    double total_area = signed_triangle_area(p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
    if (std::abs(total_area) < 1) return;

    // each pixel is drawn separately so we can take advantage of parallelism
    #pragma omp parallel for
    for (int x = xmin; x <= xmax; ++x){
        for (int y = ymin; y <= ymax; ++y){
            // these are the barycentric coordinates of the point (x, y)
            double alpha = signed_triangle_area(x, y, p2.x, p2.y, p3.x, p3.y) / total_area;
            double beta = signed_triangle_area(x, y, p3.x, p3.y, p1.x, p1.y) / total_area;
            double gamma = signed_triangle_area(x, y, p1.x, p1.y, p2.x, p2.y) / total_area;

            // check if the point falls outside the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;

            float z = alpha * p1.z + beta * p2.z + gamma * p3.z;

            // if the pixel is behind something, dont paint it
            if (z >= *_zbuffer.get(x, y)) continue;

            _zbuffer.set(x, y, {z});
            _fragmentBuffer.set(x, y, {x, y, color.asVec3()});
        }
    }
}