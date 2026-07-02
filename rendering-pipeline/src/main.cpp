#include <SDL3/SDL.h>
#include <memory>
#include <iostream>

#include "math/vec3.h"
#include "window.h"
#include "renderer.h"
#include "scene.h"
#include "geometry.h"
#include "material.h"
#include "mesh.h"
#include "color.h"
#include "camera.h"

/////////////////////////////////////////////////////////////////////////
//                           Aplication Logic                          //
/////////////////////////////////////////////////////////////////////////

const int WIDTH  = 800;
const int HEIGHT = 600;

Scene scene = Scene();

void createObjects() {
    Material mat = DiffuseMaterial(Color(0xFF0000FF), 0.40, 1);
    Material mat2 = DiffuseMaterial(Color(0xFF00FF00), 0.40, 1);

    Geometry::Cube bigCube = Geometry::Cube(Vec3(2, 0, -3), 1);
    Geometry::Cube smallCube = Geometry::Cube(Vec3(2, -0.25, -2.25), 0.5);
    Geometry::Sphere sphere = Geometry::Sphere(Vec3(0, 0, -2), 1, 32);

    Mesh bcMesh = Mesh(bigCube, mat);
    scene.add(bcMesh);
    Mesh scMesh = Mesh(smallCube, mat2);
    scene.add(scMesh);
    Mesh sMesh = Mesh(sphere, mat);
    scene.add(sMesh);
}

void createCameras() {
    float aspect = (float)WIDTH / (float)HEIGHT;
    float halfHeight = 10;
    float halfWidth = halfHeight * aspect;
    Vec3 pos = Vec3(0, 0, 0);

    std::unique_ptr<Camera> camera1 = std::make_unique<PerspectiveCamera>(pos, 90, aspect, 0.1, 100);
    scene.add(std::move(camera1));

    std::unique_ptr<Camera> camera2 = std::make_unique<OrthographicCamera>(
        pos,
        -halfWidth, halfWidth,
        halfHeight, -halfHeight,
        0.1, 100
    );
    scene.add(std::move(camera2));
}

void createLights() {
    std::unique_ptr<Light> ambient_l = std::make_unique<AmbientLight>(Color(0xffffff));
    scene.add(std::move(ambient_l));

    Vec3 pos1 = Vec3(5, 5, -1.5);
    std::unique_ptr<Light> light1 = std::make_unique<PointLight>(pos1, Color(0xff0000));
    scene.add(std::move(light1));

    Vec3 pos2 = Vec3(0, -5, -1.5);
    std::unique_ptr<Light> light2 = std::make_unique<PointLight>(pos2, Color(0xffffff));
    scene.add(std::move(light2));
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    Window window("Renderer", WIDTH, HEIGHT);
    uint32_t framebuffer[WIDTH * HEIGHT];
    Renderer renderer(&window);
    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window.getSDLWindow(), NULL);
    SDL_Texture* texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT
    );

    bool running = true;
    SDL_Event event;
    int cameraIndex = 0;

    createObjects();
    createCameras();
    createLights();

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
            if (event.type == SDL_EVENT_KEY_DOWN) {
                if (event.key.key == SDLK_ESCAPE) {
                    running = false;
                }
                if (event.key.key == SDLK_SPACE) {
                    cameraIndex = (cameraIndex + 1) % scene.cameraCount();
                }
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                float factor = (event.wheel.y < 0) ? 0.9 : 1.1;
                scene.getCamera(cameraIndex).zoom(factor);
            }
        }

        // clear to black
        memset(framebuffer, 0, sizeof(framebuffer));

        renderer.render(scene, scene.getCamera(cameraIndex));
        Buffer<Fragment> fragmentBuffer = renderer.fragmentBuffer();

        // write pixels to framebuffer
        for (int y = 0; y < HEIGHT; ++y) {
            for (int x = 0; x < WIDTH; ++x) {
                int index = (HEIGHT - 1 - y) * WIDTH + x; // flip y coordinate
                const Fragment& frag = *fragmentBuffer.get(x, y);
                // only write fragments that have been set (non-black color)
                if (frag.color.x > 0 || frag.color.y > 0 || frag.color.z > 0) {
                    framebuffer[index] = Color::fromVec3(frag.color).value();
                }
            }
        }

        SDL_UpdateTexture(texture, NULL, framebuffer, WIDTH * sizeof(uint32_t));
        SDL_RenderClear(sdl_renderer);
        SDL_RenderTexture(sdl_renderer, texture, NULL, NULL);
        SDL_RenderPresent(sdl_renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_Quit();
    return 0;
}