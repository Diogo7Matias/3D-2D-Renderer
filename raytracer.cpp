#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>

#define MAX_RAY_DEPTH 5 // max recursion depth allowed

// 3D Vector class
template<typename T> class Vec3 {
public:
    T x, y, z;

    // constructors
    Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
    Vec3(T a) : x(a), y(a), z(a) {}
    Vec3(T a, T b, T c) : x(a), y(b), z(c) {}
    
    // returns the vector but with magnitude = 1
    Vec3& normalize(){
        T length = sqrt(x*x + y*y + z*z);
        if (length > 0){
            x /= length;
            y /= length;
            z /= length;
        }
        return *this;
    }

    // dot product
    T dot(const Vec3<T> &v) const {return x * v.x + y * v.y + z * v.z;}
    // operator overloads
    Vec3<T> operator + (const Vec3<T> &v) const {return Vec3<T>(x + v.x, y + v.y, z + v.z);}
    Vec3<T> operator - (const Vec3<T> &v) const {return Vec3<T>(x - v.x, y - v.y, z - v.z);}
    Vec3<T> operator * (const T &k) const {return Vec3<T>(x * k, y * k, z * k);}
    Vec3<T> operator * (const Vec3<T> &v) const {return Vec3<T>(x * v.x, y * v.y, z * v.z);}
    Vec3<T>& operator += (const Vec3<T> &v) {x += v.x, y += v.y, z += v.z; return *this;}
    Vec3<T>& operator *= (const Vec3<T> &v) {x *= v.x, y *= v.y, z *= v.z; return *this;}
    Vec3<T> operator - () const {return Vec3<T>(-x, -y, -z);}

    friend std::ostream& operator << (std::ostream &os, const Vec3<T> &v){
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};

typedef Vec3<float> Vec3f;

class Sphere {
public:
    Vec3f center;
    float radius;
    Vec3f surfaceColor, emissionColor;
    float transparency, reflection;

    // constructor
    Sphere(
        const Vec3f &c,
        const float &r,
        const Vec3f &sc,
        const float &transp = 0,
        const float &refl = 0,
        const Vec3f &ec = 0
    ) : center(c), radius(r), surfaceColor(sc), emissionColor(ec), transparency(transp), reflection(refl) {}

    // checks if a ray with origin 'rayOrig' and direction 'rayDir' intersects the sphere
    // the points in the ray where it enters and leaves the sphere are stored in t0 and t1, respectively
    bool intersect(const Vec3f &rayOrig, const Vec3f &rayDir, float &t0, float &t1) const {
        Vec3f l = center - rayOrig;
        float tca = l.dot(rayDir); // distance given by projecting l onto ray
        if (tca < 0) return false; // ray is pointing away from the sphere
        float d2 = l.dot(l) - tca * tca; // perpendicular distance from center to ray squared
        if (d2 > radius * radius) return false; // ray is completely outside the sphere
        float thc = sqrt(radius * radius - d2); // half of the part of the ray that passes through the sphere
        // now we find the intersections 
        t0 = tca - thc; 
        t1 = tca + thc;
        return true;
    }
};

float lerp(const float &a, const float &b, const float &mix){
    return b * mix + a * (1 - mix);
}

// Checks if a given ray intersects any objects in the scene and shades the intersection points accordingly
Vec3f trace(const Vec3f &rayOrig, const Vec3f &rayDir, const std::vector<Sphere> &spheres, const int &depth){
    float tnear = INFINITY;
    const Sphere* sphere = NULL;

    // find intersections and determine the closest one
    for (unsigned i = 0; i < spheres.size(); i++){
        float t0 = INFINITY;
        float t1 = INFINITY;

        if (spheres[i].intersect(rayOrig, rayDir, t0, t1)){
            if (t0 < 0) t0 = t1; // if the ray starts inside the sphere
            if (t0 < tnear){ // store closest intersection
                tnear = t0;
                sphere = &spheres[i];
            }
        }
    }

    if (!sphere) return Vec3f(2); // no intersections --> background color

    Vec3f surfaceColor = 0; // color of the object
    Vec3f pInt = rayOrig + rayDir * tnear; // point (coordinates) of intersection
    Vec3f nInt = pInt - sphere->center; // normal at the point of intersection
    nInt.normalize();

    float bias = 1e-4;
    bool inside = false;

    // if the directions of the ray and normal are not opposite, we are inside the sphere 
    if (rayDir.dot(nInt) > 0){
        nInt = -nInt;
        inside = true;
    }

    if ((sphere->transparency > 0 || sphere->reflection > 0) && depth < MAX_RAY_DEPTH){
        float facingRatio = -rayDir.dot(nInt);
        float fresnelEffect = lerp(pow(1 - facingRatio, 3), 1, 0.1);
        Vec3f reflDir = rayDir - nInt * 2 * rayDir.dot(nInt);
        reflDir.normalize();
        Vec3f reflexion = trace(pInt + nInt * bias, reflDir, spheres, depth + 1);
        Vec3f refraction = 0;

        // compute refraction ray
        if (sphere->transparency){
            float indexOfRefraction = 1.1;
            float eta = (inside) ? indexOfRefraction : 1 / indexOfRefraction; // ratio of refractive indices
            float cosi = -nInt.dot(rayDir); // angle between surface normal and incoming ray
            float k = 1 - eta * eta * (1 - cosi * cosi);
            Vec3f refrDir = rayDir * eta + nInt * (eta * cosi - sqrt(k));
            refrDir.normalize();
            refraction = trace(pInt - nInt * bias, refrDir, spheres, depth + 1);
        }

        // final result
        surfaceColor = (reflexion * fresnelEffect + refraction *
            (1 - fresnelEffect) * sphere->transparency) *sphere->surfaceColor;
    } else {
        // it's a diffuse object
        for (unsigned i = 0; i < spheres.size(); i++){
            // loop through light sources, cast shadow rays and check for objects in the way
            if (spheres[i].emissionColor.x > 0){
                Vec3f transmission = 1;
                Vec3f lightDir = spheres[i].center - pInt;
                lightDir.normalize();

                for (unsigned j = 0; j < spheres.size(); j++){
                    if (i != j){
                        float t0, t1;
                        if (spheres[j].intersect(pInt + nInt * bias, lightDir, t0, t1)){
                            transmission = 0; // light is blocked
                            break;
                        }
                    }
                }
                surfaceColor += sphere->surfaceColor * transmission *
                std::max(float(0), nInt.dot(lightDir)) * spheres[i].emissionColor;
            }
        }
    }

    return surfaceColor + sphere->emissionColor;
}

void render(const std::vector<Sphere> &spheres){
    unsigned width = 1920, height = 1080;
    float aspectRatio = width / float(height);
    Vec3f* image = new Vec3f[width *  height];
    Vec3f* pixel = image;
    float fov = 30;
    float angle = tan(M_PI / 2 * fov / 180.);

    // perspective projection
    for (unsigned y = 0; y < height; y++){
        for (unsigned x = 0; x < width; x++){
            // normalize coordinates
            float xx = (2 * ((x + 0.5) / float(width)) - 1) * angle * aspectRatio;
            float yy = (1 - 2 * ((y + 0.5) / float(height))) * angle;
            
            // define direction and trace
            Vec3f rayDir(xx, yy, -1);
            rayDir.normalize();
            *pixel = trace(Vec3f(0), rayDir, spheres, 0);
            pixel++;
        }
    }

    // save to PPM image
    std::ofstream ofs("./result.ppm", std::ios::out | std::ios::binary);
    ofs << "P6\n" << width << " " << height << "\n255\n"; // header
    for (unsigned i = 0; i < width * height; i++){ // rest of the file
        ofs << (unsigned char)(std::min(float(1), image[i].x) * 255) <<
            (unsigned char)(std::min(float(1), image[i].y) * 255) <<
            (unsigned char)(std::min(float(1), image[i].z) * 255);
    }
    ofs.close();
    delete [] image; // deallocate pixel array
}

int main(int argc, char** argv){
    std::vector<Sphere> spheres;

    // first sphere acts as the ground
    spheres.push_back(Sphere(Vec3f(0, -10004, -20), 10000, Vec3f(0.20, 0.20, 0.20), 0.0, 0));
    // red sphere
    spheres.push_back(Sphere(Vec3f(2, 1, -40), 5, Vec3f(1.00, 0.32, 0.36), 0.2, 1));
    // yellow sphere
    spheres.push_back(Sphere(Vec3f(5, -2, -25), 2, Vec3f(0.98, 0.73, 0.01), 0.5, 1));
    // floating sphere
    spheres.push_back(Sphere(Vec3f(7, 4, -19), 3, Vec3f(0.98, 0.73, 0.01), 0.2, 1));
    // light blue sphere
    spheres.push_back(Sphere(Vec3f(-2, 0, -30), 4, Vec3f(0.30, 0.78, 1.00), 0.2, 1));
    // dark sphere
    spheres.push_back(Sphere(Vec3f(-6, 0, -20), 4, Vec3f(0.15, 0.15, 0.15), 0.0, 1));
    // light sources
    spheres.push_back(Sphere(Vec3f(10, 20, -10), 3, Vec3f(0.00, 0.00, 0.00), 0.0, 0, Vec3f(5)));
    spheres.push_back(Sphere(Vec3f(-3, 20, -5), 3, Vec3f(0.00, 0.00, 0.00), 0.0, 0, Vec3f(3)));

    render(spheres);
    return 0;
}