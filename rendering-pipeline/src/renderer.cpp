#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <optional>

#include "renderer.h"
#include "math/vec4.h"
#include "math/mat4.h"

void Renderer::render(const Scene &scene, const Camera &camera) {
    std::vector<Material> materials = scene.materials();
    std::vector<Light*> lights = scene.getLights();
    
    _fragmentBuffer.clear({0, 0, _background.asVec3()});
    _zbuffer.clear(std::numeric_limits<float>::infinity());

    Mat4 view = camera.viewMatrix();
    Mat4 projection = camera.projectionMatrix();
    Mat4 viewport = _window->viewportMatrix();

    // Transform lights to view space (camera coordinates)
    for (Light* light : lights) {
        if (PointLight* pl = dynamic_cast<PointLight*>(light)) {
            pl->setViewPosition(view);
        }
    }

    for (Mesh& sceneObject : scene.objects()) {
        Mat4 translate = Mat4::translation(sceneObject.position());
        Mat4 rotate = Mat4::rotation(sceneObject.rotation().x, sceneObject.rotation().y, sceneObject.rotation().z);
        Mat4 scale = Mat4::scale(sceneObject.scale());
        Mat4 model = translate * rotate * scale;
        Mat4 viewModel = view * model;
        Mat4 normalMatrix = viewModel.inverseTranspose();
        
        const Material& material = sceneObject.material();
        std::vector<Vertex> vertices;

        // Transform vertices and normals from object local space to view space
        for (const auto &vertex : sceneObject.geometry().getVertices()) {
            Vertex v = vertex;
            v.position = (viewModel * v.position.toVec4()).toVec3();
            
            Vec4 n = normalMatrix * Vec4(v.normal.x, v.normal.y, v.normal.z, 0.0f);
            v.normal = Vec3(n.x, n.y, n.z).normalize();
            vertices.push_back(v);
        }

        // Calculate vertex colors, project coordinates, clip points outside frustum 
        // and pass result to fragment buffer
        if (_wireframe) {
            for (const auto& edge : sceneObject.geometry().getEdges()) {
                const Vertex &v0 = vertices[edge.first];
                const Vertex &v1 = vertices[edge.second];
                
                Vec4 proj0 = projection * v0.position.toVec4();
                Vec4 proj1 = projection * v1.position.toVec4();

                if (!clipLine(proj0, proj1)) continue;
                
                Vec3 ndc0 = proj0.toVec3();
                Vec3 ndc1 = proj1.toVec3();
                
                Vec4 p0 = viewport * ndc0.toVec4();
                Vec4 p1 = viewport * ndc1.toVec4();
                
                Color color = material.getColor();
                line(Vec3(p0.x, p0.y, p0.z), Vec3(p1.x, p1.y, p1.z), ndc0.z, ndc1.z, color);
            }
        } else {
            for (const Triangle& t : sceneObject.geometry().getTriangles()) {
                Vertex& v0 = vertices[t.v0];
                Vertex& v1 = vertices[t.v1];
                Vertex& v2 = vertices[t.v2];

                if (isBackFace(t, vertices)) continue;
                
                if (!_depthShading) {
                    computeLighting(v0, v1, v2, material, lights);
                }

                Vec4 proj0 = projection * v0.position.toVec4();
                Vec4 proj1 = projection * v1.position.toVec4();
                Vec4 proj2 = projection * v2.position.toVec4();

                std::vector<Vec4> polygon = clipTriangle(proj0, proj1, proj2);

                // after clipping, triangle might get transformed to another polygon.
                // therefore -> triangulate as fan
                for (size_t i = 1; i + 1 < polygon.size(); ++i) {
                    Vec4 c0 = polygon[0];
                    Vec4 c1 = polygon[i];
                    Vec4 c2 = polygon[i + 1];

                    Vec3 ndc0 = c0.toVec3();
                    Vec3 ndc1 = c1.toVec3();
                    Vec3 ndc2 = c2.toVec3();

                    Vec4 p0 = (viewport * ndc0.toVec4());
                    Vec4 p1 = (viewport * ndc1.toVec4());
                    Vec4 p2 = (viewport * ndc2.toVec4());

                    Vertex v0cpy = Vertex({p0.x, p0.y, p0.z}, v0.normal, v0.color);
                    Vertex v1cpy = Vertex({p1.x, p1.y, p1.z}, v1.normal, v1.color);
                    Vertex v2cpy = Vertex({p2.x, p2.y, p2.z}, v2.normal, v2.color);

                    triangle(v0cpy, v1cpy, v2cpy, ndc0.z, ndc1.z, ndc2.z, material);
                }
            }
        }
    }
}

bool Renderer::isBackFace(const Triangle &t, const std::vector<Vertex> &vertices) {
    const Vec3& p0 = vertices[t.v0].position;
    const Vec3& p1 = vertices[t.v1].position;
    const Vec3& p2 = vertices[t.v2].position;

    Vec3 edge1 = p1 - p0;
    Vec3 edge2 = p2 - p0;

    Vec3 normal = edge1.cross(edge2).normalize();
    return normal.z <= 0;
}

// implements the Sutherland-Hodgman clipping algorithm for lines 
bool Renderer::clipLine(Vec4& a, Vec4& b) {
    auto clipLineAgainstPlane = [&a, &b](float da, float db) -> bool {
        bool aInside = da >= 0.0f;
        bool bInside = db >= 0.0f;

        if (!aInside && !bInside)
            return false;

        if (aInside != bInside) {
            float t = da / (da - db);

            Vec4 intersection = a + (b - a) * t;

            if (!aInside)
                a = intersection;
            else
                b = intersection;
        }

        return true;
    };

    return not (!clipLineAgainstPlane(a.x + a.w, b.x + b.w) // left
        || !clipLineAgainstPlane(a.w - a.x, b.w - b.x)      // right
        || !clipLineAgainstPlane(a.y + a.w, b.y + b.w)      // bottom
        || !clipLineAgainstPlane(a.w - a.y, b.w - b.y)      // top
        || !clipLineAgainstPlane(a.z + a.w, b.z + b.w)      // near
        || !clipLineAgainstPlane(a.w - a.z, b.w - b.z));    // far
}

// implements the Sutherland-Hodgman polygon clipping algorithm for triangles 
std::vector<Vec4> Renderer::clipTriangle(const Vec4& v0, const Vec4& v1, const Vec4& v2) {
    std::vector<Vec4> polygon = {v0, v1, v2};

    for (int plane = 0; plane < 6; ++plane) {
        if (polygon.empty())
            break;

        std::vector<Vec4> clipped;
        Vec4 previous = polygon.back();

        auto getDistance = [plane](const Vec4& v) {
            switch (plane) {
                case 0: return v.x + v.w; // left
                case 1: return v.w - v.x; // right
                case 2: return v.y + v.w; // bottom
                case 3: return v.w - v.y; // top
                case 4: return v.z + v.w; // near
                case 5: return v.w - v.z; // far
                default: return 0.0f;
            }
        };

        float previousDistance = getDistance(previous);
        bool previousInside = previousDistance >= 0.0f;

        for (const Vec4& current : polygon) {
            float currentDistance = getDistance(current);
            bool currentInside = currentDistance >= 0.0f;

            if (currentInside != previousInside) {
                float t = previousDistance / (previousDistance - currentDistance);
                Vec4 intersection = previous + (current - previous) * t;
                clipped.push_back(intersection);
            }

            if (currentInside)
                clipped.push_back(current);

            previous = current;
            previousDistance = currentDistance;
            previousInside = currentInside;
        }

        polygon = std::move(clipped);
    }

    return polygon;
}

void Renderer::computeLighting(Vertex &v0, Vertex &v1, Vertex &v2, const Material &material, const std::vector<Light*>& lights) {
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

std::optional<Mesh> Renderer::loadOBJModel(const std::string& filename, const Material& material) {
    std::ifstream objFile(filename);

    if (!objFile.is_open()) {
        std::cerr << "Error: Could not open OBJ file: " << filename << std::endl;
        return std::nullopt;
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
        
        // NOTE: currently ignores texture coordinates (vt)
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
            // parses face vertex token which can be: v, v/vt, v//vn or v/vt/vn
            auto parseFaceVertex = [](const std::string& token) -> std::pair<int,int> {
                std::vector<std::string> parts;
                std::string part;
                std::istringstream ss(token);
                
                while (std::getline(ss, part, '/')) parts.push_back(part);

                int v_idx = -1;
                int vn_idx = -1;

                if (parts.size() >= 1 && !parts[0].empty()) {
                    v_idx = std::stoi(parts[0]) - 1; // OBJ indices are 1-based
                }
                if (parts.size() == 3 && !parts[2].empty()) {
                    vn_idx = std::stoi(parts[2]) - 1;
                }
                return {v_idx, vn_idx};
            };

            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) tokens.push_back(token);

            // triangulate as fan:
            // for polygons with > 3 vertices, divide into triangles using the first vertex as a common point
            for (size_t i = 1; i + 1 < tokens.size(); ++i) {
                auto p1 = parseFaceVertex(tokens[0]);
                auto p2 = parseFaceVertex(tokens[i]);
                auto p3 = parseFaceVertex(tokens[i + 1]);

                int v1_idx = p1.first, v2_idx = p2.first, v3_idx = p3.first;
                int vn1_idx = p1.second, vn2_idx = p2.second, vn3_idx = p3.second;

                // validate position indices
                // NOTE: negative (relative) OBJ indices are not supported
                if (v1_idx < 0 || v2_idx < 0 || v3_idx < 0 ||
                    v1_idx >= (int)positions.size() || v2_idx >= (int)positions.size() || v3_idx >= (int)positions.size()) {
                    std::cerr << "Warning: face references invalid vertex index in " << filename << std::endl;
                    continue;
                }

                Vec3 v1_pos = positions[v1_idx];
                Vec3 v2_pos = positions[v2_idx];
                Vec3 v3_pos = positions[v3_idx];

                // calculate face normal if normals weren't provided for this face
                Vec3 edge1 = v2_pos - v1_pos;
                Vec3 edge2 = v3_pos - v1_pos;
                Vec3 faceNormal = edge1.cross(edge2).normalize();

                // choose normals: prefer explicit vn index, otherwise fall back to face normal
                Vec3 v1_normal = faceNormal;
                Vec3 v2_normal = faceNormal;
                Vec3 v3_normal = faceNormal;
                if (vn1_idx >= 0 && vn1_idx < (int)normals.size()) v1_normal = normals[vn1_idx];
                if (vn2_idx >= 0 && vn2_idx < (int)normals.size()) v2_normal = normals[vn2_idx];
                if (vn3_idx >= 0 && vn3_idx < (int)normals.size()) v3_normal = normals[vn3_idx];

                // add vertices
                vertices.push_back(Vertex(v1_pos, v1_normal, -1));
                vertices.push_back(Vertex(v2_pos, v2_normal, -1));
                vertices.push_back(Vertex(v3_pos, v3_normal, -1));

                triangles.push_back(Triangle(vertexCount, vertexCount + 1, vertexCount + 2));
                vertexCount += 3;
            }
        }
    }

    return Mesh(Geometry::OBJModel(vertices, triangles), material);
}