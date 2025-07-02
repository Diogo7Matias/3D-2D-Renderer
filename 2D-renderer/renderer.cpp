#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "tgaimage.h"
#include "cmath"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

int width = 1200;
int height = 1200;

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

            // draw lines for every pair of verticess
            line(ax, ay, bx, by, image, green);
            line(bx, by, cx, cy, image, green);
            line(cx, cy, ax, ay, image, green);
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
	image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}


