#include "tgaimage.h"
#include "cmath"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red = TGAColor(255, 0, 0, 255);
const TGAColor green = TGAColor(0, 255, 0, 255);
const TGAColor blue = TGAColor(0, 0, 255, 255);

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

int main(int argc, char const *argv[]){
	TGAImage image(64, 64, TGAImage::RGB);
    // line(13, 20, 80, 40, image, white);
    // line(13, 20, 5, 80, image, white);

    int ax = 7, ay = 3;
    int bx = 12, by = 37;
    int cx = 62, cy = 53; 
    
    // lines
    line(ax, ay, bx, by, image, blue);
    line(cx, cy, bx, by, image, green);
    line(ax, ay, cx, cy, image, red);
    
    // points
    image.set(ax, ay, white);
    image.set(bx, by, white);
    image.set(cx, cy, white);

	image.flip_vertically();
    image.write_tga_file("output.tga");
    return 0;
}


