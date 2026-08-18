#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "renderer.h"
#include "math/vec4.h"
#include "math/mat4.h"

void Renderer::render(const Scene &scene, const Camera &camera) {
    std::vector<Vertex> vertices = scene.vertices();
    std::vector<Material> materials = scene.materials();
    std::vector<Light*> lights = scene.getLights();
    
    _fragmentBuffer.clear({0, 0, _background.asVec3()});
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

    // Calculate vertex colors, project coordinates, clip points outside frustum 
    // and pass result to fragment buffer
    if (_wireframe) {
        for (const auto& edge : scene.edges()) {
            Vertex &v0 = vertices[edge.first];
            Vertex &v1 = vertices[edge.second];
            Vec4 proj0 = clipAndProject(v0.position, projection).toVec4();
            Vec4 proj1 = clipAndProject(v1.position, projection).toVec4();
            Vec3 p0 = (viewport * proj0).toVec3();
            Vec3 p1 = (viewport * proj1).toVec3();
            Color color = materials[v0.materialIndex].getColor();
            line(p0, p1, proj0.z, proj1.z, color);
        }
    } else {
        for (const Triangle& t : scene.triangles()) {
            Vertex& v0 = vertices[t.v0];
            Vertex& v1 = vertices[t.v1];
            Vertex& v2 = vertices[t.v2];
            Material& material = materials[v0.materialIndex];
    
            if (isBackFace(t, vertices)) continue;
            
            if (!_depthShading) {
                computeLighting(v0, v1, v2, material, lights);
            }
    
            Vec4 proj0 = clipAndProject(v0.position, projection).toVec4();
            Vec4 proj1 = clipAndProject(v1.position, projection).toVec4();
            Vec4 proj2 = clipAndProject(v2.position, projection).toVec4();
            
            Vec3 p0 = (viewport * proj0).toVec3();
            Vec3 p1 = (viewport * proj1).toVec3();
            Vec3 p2 = (viewport * proj2).toVec3();
            
            Vertex v0cpy = Vertex(p0, v0.normal, v0.color, v0.materialIndex);
            Vertex v1cpy = Vertex(p1, v1.normal, v1.color, v1.materialIndex);
            Vertex v2cpy = Vertex(p2, v2.normal, v2.color, v2.materialIndex);

            triangle(v0cpy, v1cpy, v2cpy, proj0.z, proj1.z, proj2.z, material);
        }
    }
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

void Renderer::computeLighting(Vertex &v0, Vertex &v1, Vertex &v2, const Material &material, const std::vector<Light*> lights) {
    if (lights.size() > 0) {
        v0.color = v1.color = v2.color = material.getColor();
    }

    switch (material.getShading()) {
        case FLAT: {
            Vec3 edge1 = v1.position - v0.position;
            Vec3 edge2 = v2.position - v0.position;
            Vec3 normal = edge1.cross(edge2).normalize();

            for (Light* light : lights) {
                v0.color += light->compute(v0.position, normal, material);
                v1.color += light->compute(v1.position, normal, material);
                v2.color += light->compute(v2.position, normal, material);
            }
            break;
        }
        case GOURAUD: {
            for (Light* light : lights) {
                v0.color += light->compute(v0.position, v0.normal, material);
                v1.color += light->compute(v1.position, v1.normal, material);
                v2.color += light->compute(v2.position, v2.normal, material);
            }
            break;
        }
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
void Renderer::line(const Vec3 &p0, const Vec3 &p1, float z0, float z1, const Color &color){
    // parameterization of the segment [A, B] where A = (x0, y0), B = (x1, y1):
    //      x(t) = x0 + t * (x1 - x0)
    //      y(t) = y0 + t * (y1 - y0)
    // t is a parameter between 0 and 1.

    int x0 = (int)p0.x, y0 = (int)p0.y, x1 = (int)p1.x, y1 = (int)p1.y;

    if (x0 == x1 && y0 == y1) {
        Color shadedColor = applyDepthShading(color, z0);
        _fragmentBuffer.set(x0, y0, {x0, y0, shadedColor.asVec3()});
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
        std::swap(z0, z1);
    }

    // main component of the algorithm where we iterate over x to draw the line
    for (int x = x0; x <= x1; x++){
        float t = (x - x0) / (float)(x1 - x0);
        int y = std::round(y0 * (1. - t) + y1 * t);
        float z = z0 * (1. - t) + z1 * t;
        Color shadedColor = applyDepthShading(color, z);

        // check if we had previously transposed the coordinates and de-transpose if so
        if (steep){
            _fragmentBuffer.set(y, x, {y, x, shadedColor.asVec3()});
        } else {
            _fragmentBuffer.set(x, y, {x, y, shadedColor.asVec3()});
        }
    }
}

Color Renderer::applyDepthShading(const Color &color, float z) const {
    if (!_depthShading) return color;
    float normalizedDepth = (z + 1.0f) / 2.0f;
    float intensity = 1.0f - normalizedDepth * 0.8f;
    return Color::fromVec3(Vec3(1.0f, 1.0f, 1.0f) * intensity);
}

double signed_triangle_area(float x0, float y0, float x1, float y1, float x2, float y2){
    return ((x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0)) / 2.0;
}

void Renderer::triangle(const Vertex &v1, const Vertex &v2, const Vertex &v3, float z1, float z2, float z3, const Material &material) {
    Vec3 p1 = v1.position;
    Vec3 p2 = v2.position;
    Vec3 p3 = v3.position;
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

            float z = alpha * z1 + beta * z2 + gamma * z3;

            // if the pixel is behind something, dont paint it
            if (z >= *_zbuffer.get(x, y)) continue;
            _zbuffer.set(x, y, {z});

            Color pixelColor = v1.color;
            if (material.getShading() == GOURAUD) {
                pixelColor = Color::fromVec3(Vec3(
                    alpha * v1.color.red() + beta * v2.color.red() + gamma * v3.color.red(),
                    alpha * v1.color.green() + beta * v2.color.green() + gamma * v3.color.green(),
                    alpha * v1.color.blue() + beta * v2.color.blue() + gamma * v3.color.blue()
                ));
            }

            Color shadedColor = applyDepthShading(pixelColor, z);
            setFragmentColor(Vec3(x, y, z), shadedColor);
        }
    }
}

Mesh Renderer::loadOBJModel(const std::string& filename, const Material& material) {
    std::ifstream objFile(filename);

    if (!objFile.is_open()) {
        std::cerr << "Error: Could not open OBJ file: " << filename << std::endl;
        return Mesh(Geometry::Cube(0.1f), material); // return a dummy mesh
    }

    std::vector<Vec3> positions;
    std::vector<Vec3> normals;
    std::vector<Vertex> vertices;
    std::vector<Triangle> triangles;
    std::string line;
    
    int vertexCount = 0;

    while (std::getline(objFile, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string type;
        iss >> type;

        if (type == "v") { // vertex position
            float x, y, z;
            iss >> x >> y >> z;
            positions.push_back(Vec3(x, y, z));
        } 
        else if (type == "vn") { // vertex normal
            float x, y, z;
            iss >> x >> y >> z;
            normals.push_back(Vec3(x, y, z).normalize());
        }
        else if (type == "f") { // face

            // WARNING: this next segment was vibe coded and may not be working properly
            // TODO: check for correctness
            
            std::string v1_str, v2_str, v3_str;
            iss >> v1_str >> v2_str >> v3_str;

            // Parse face vertex token which can be: v, v/vt, v//vn or v/vt/vn
            auto parseFaceVertex = [](const std::string& token) -> std::pair<int,int> {
                std::vector<std::string> parts;
                std::string part;
                std::istringstream ss(token);
                while (std::getline(ss, part, '/')) parts.push_back(part);
                int vIdx = -1;
                int vnIdx = -1;
                if (parts.size() >= 1 && !parts[0].empty()) {
                    vIdx = std::stoi(parts[0]) - 1; // OBJ indices are 1-based
                }
                if (parts.size() == 3 && !parts[2].empty()) {
                    vnIdx = std::stoi(parts[2]) - 1;
                }
                return {vIdx, vnIdx};
            };

            auto p1 = parseFaceVertex(v1_str);
            auto p2 = parseFaceVertex(v2_str);
            auto p3 = parseFaceVertex(v3_str);

            int v1_idx = p1.first, v2_idx = p2.first, v3_idx = p3.first;
            int v1_n_idx = p1.second, v2_n_idx = p2.second, v3_n_idx = p3.second;

            // Validate position indices
            if (v1_idx < 0 || v2_idx < 0 || v3_idx < 0 ||
                v1_idx >= (int)positions.size() || v2_idx >= (int)positions.size() || v3_idx >= (int)positions.size()) {
                std::cerr << "Warning: face references invalid vertex index in " << filename << std::endl;
                continue;
            }

            Vec3 v1_pos = positions[v1_idx];
            Vec3 v2_pos = positions[v2_idx];
            Vec3 v3_pos = positions[v3_idx];

            // Calculate face normal if normals weren't provided for this face
            Vec3 edge1 = v2_pos - v1_pos;
            Vec3 edge2 = v3_pos - v1_pos;
            Vec3 faceNormal = edge1.cross(edge2).normalize();

            // Choose normals: prefer explicit vn index, otherwise fall back to face normal
            Vec3 v1_normal = faceNormal;
            Vec3 v2_normal = faceNormal;
            Vec3 v3_normal = faceNormal;
            if (v1_n_idx >= 0 && v1_n_idx < (int)normals.size()) v1_normal = normals[v1_n_idx];
            if (v2_n_idx >= 0 && v2_n_idx < (int)normals.size()) v2_normal = normals[v2_n_idx];
            if (v3_n_idx >= 0 && v3_n_idx < (int)normals.size()) v3_normal = normals[v3_n_idx];

            // Add vertices
            Vertex vert1(v1_pos, v1_normal, Color(0xFFFFFF), 0);
            Vertex vert2(v2_pos, v2_normal, Color(0xFFFFFF), 0);
            Vertex vert3(v3_pos, v3_normal, Color(0xFFFFFF), 0);

            vertices.push_back(vert1);
            vertices.push_back(vert2);
            vertices.push_back(vert3);

            // Add triangle (indices are relative to this mesh)
            triangles.push_back(Triangle(vertexCount, vertexCount + 1, vertexCount + 2));
            vertexCount += 3;
        }
    }

    objFile.close();

    return Mesh(Geometry::OBJModel(vertices, triangles), material);
}