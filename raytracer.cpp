#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <cassert>

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

    // operator overloads
    Vec3<T> operator + (const Vec3<T> &v) const {return Vec3<T>(x + v.x, y + v.y, z + v.z);}
    Vec3<T> operator - (const Vec3<T> &v) const {return Vec3<T>(x - v.x, y - v.y, z - v.z);}
    Vec3<T> operator * (const T &k) const {return Vec3<T>(x * k, y * k, z * k);}
    Vec3<T> operator * (const Vec3<T> &v) const (return Vec3<T>(x * v.x, y * v.y, z * v.z);)
    Vec3<T>& operator += (const Vec3<T> &v) {x += v.x, y += v.y, z += v.z; return *this;}
    Vec3<T>& operator *= (const Vec3<T> &v) {x *= v.x, y *= v.y, z *= v.z; return *this;}
    Vec3<T> operator - () const {return Vec3<T>(-x, -y, -z);}
    // dot product
    T dot(const Vec3<T> &v) const {return x * v.x + y * v.y + z * v.z;}

    std::ostream& operator << (std::ostream &os, const Vec3<T> &v){
        os << "[" << v.x << " " << v.y << " " << v.z << "]";
        return os;
    }
};
