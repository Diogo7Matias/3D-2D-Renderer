#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

#define MAX_RAY_DEPTH 5; // max recursion depth allowed

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
    Vec3<T> operator * (const Vec3<T> &v) const (return Vec3<T>(x * v.x, y * v.y, z * v.z);)
    Vec3<T>& operator += (const Vec3<T> &v) {x += v.x, y += v.y, z += v.z; return *this;}
    Vec3<T>& operator *= (const Vec3<T> &v) {x *= v.x, y *= v.y, z *= v.z; return *this;}
    Vec3<T> operator - () const {return Vec3<T>(-x, -y, -z);}

    std::ostream& operator << (std::ostream &os, const Vec3<T> &v){
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
    bool intersect(const Vec3f &rayOrig, const Vec3f &rayDir, float &t0, float &t1){
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
    // NOT IMPLEMENTED
}