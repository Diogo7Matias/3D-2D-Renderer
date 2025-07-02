#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "tgaimage.h"
#include "cmath"

constexpr TGAColor white = {255, 255, 255, 255};
constexpr TGAColor red = {0, 0, 255, 255};
constexpr TGAColor green = {0, 255, 0, 255};
constexpr TGAColor blue = {255, 0, 0, 255};

int width = 1300;
int height = 1300;

class Point{
     // A point (or vector) in 3D space
public:
    float x, y, z;
    
    Point(float xCoord, float yCoord, float zCoord){
        x = xCoord;
        y = yCoord;
        z = zCoord;
    }
};

// this function implements the Bresenham's line drawing algorithm
void line(int x0, int y0, int x1, int y1, TGAImage &image, TGAColor color){
    // parameterization of the segment [A, B] where A = (x0, y0), B = (x1, y1):
    //      x(t) = x0 + t * (x1 - x0)
    //      y(t) = y0 + t * (y1 - y0)
    // t is a parameter between 0 and 1.

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
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

double signed_triangle_area(int x0, int y0, int x1, int y1, int x2, int y2){
    return ((y1 - y0) * (x1 + x0) + (y2 - y1) * (x2 + x1) + (y0 - y2) * (x0 + x2)) / 2;
}

void triangle(int x0, int y0, int x1, int y1, int x2, int y2, TGAImage &image, TGAColor color){
    int xmin = std::min(std::min(x0, x1), x2);
    int ymin = std::min(std::min(y0, y1), y2);
    int xmax = std::max(std::max(x0, x1), x2);
    int ymax = std::max(std::max(y0, y1), y2);
    double total_area = signed_triangle_area(x0, y0, x1, y1, x2, y2);
    if (total_area < 1) return;

    // each pixel is drawn separately so we can take advantage of parallelism
    #pragma omp parallel for
    for (int x = xmin; x <= xmax; x++){
        for (int y = ymin; y <= ymax; y++){
            // these are the barycentric coordinates of the point (x, y)
            double alpha = signed_triangle_area(x, y, x1, y1, x2, y2) / total_area;
            double beta = signed_triangle_area(x, y, x2, y2, x0, y0) / total_area;
            double gamma = signed_triangle_area(x, y, x0, y0, x1, y1) / total_area;

            // check if the point falls outside the triangle
            if (alpha < 0 || beta < 0 || gamma < 0) continue;
            image.set(x, y, color);
        }
    }
}

std::tuple<int, int> projection(Point p){
    return {(p.x + 1.) * width/2, (p.y + 1.) * height/2};
}

void renderModel(std::string filename, TGAImage &image){
    std::ifstream objFile;
    std::string fline;
    std::vector<Point> points;
    
    objFile.open(filename);
    while (std::getline(objFile, fline)){
        std::istringstream iss(fline);
        std::vector<std::string> components;
        std::string token;

        // split line by whitespace
        while (iss >> token){
            std::istringstream tokenStream(token);
            std::string component;
            
            std::getline(tokenStream, component, '/');
            components.push_back(component);
        }

        if (!components.size()) continue;

        if (components[0] == "v") { // parse a point
            Point point(stof(components[1]), stof(components[2]), stof(components[3]));
            points.push_back(point);
        } else if (components[0] == "f") { // parse a face (3 vertices)
            auto [ax, ay] = projection(points[stoi(components[1]) - 1]);
            auto [bx, by] = projection(points[stoi(components[2]) - 1]);
            auto [cx, cy] = projection(points[stoi(components[3]) - 1]);

            // render triangles with random colors
            TGAColor randColor;
            for (int c = 0; c < 3; c++) randColor[c] = std::rand() % 255;
            triangle(ax, ay, bx, by, cx, cy, image, randColor);
            // draw points
            image.set(ax, ay, white);
            image.set(bx, by, white);
            image.set(cx, cy, white);
        }
    }
    objFile.close();
}

int main(int argc, char const *argv[]){
	TGAImage image(width, height, TGAImage::RGB);

    if (argc != 2){
        std::cout << "Usage: " << argv[0] << " objmodel.obj" << std::endl;
        return 1;
    }

    renderModel(argv[1], image);

    // triangle(7, 45, 35, 100, 45, 60, image, red);
    // triangle(120, 35, 90, 5, 45, 110, image, white);
    // triangle(115, 83, 80, 90, 85, 120, image, green);

	// image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}


