#include <SDL3/SDL.h>
#include <memory>
#include <iostream>
#include <optional>

#include "math/vec3.h"
#include "window.h"
#include "renderer.h"
#include "scene.h"
#include "mesh/mesh.h"
#include "mesh/geometry.h"
#include "mesh/material.h"
#include "utils/color.h"
#include "camera.h"

/////////////////////////////////////////////////////////////////////////
//                           Aplication Logic                          //
/////////////////////////////////////////////////////////////////////////

const int WIDTH  = 800;
const int HEIGHT = 600;

Window window("Renderer", WIDTH, HEIGHT);
Renderer renderer(&window);
Scene scene = Scene();

int cameraIndex = 0;
bool wireframe = false;
bool depthShading = false;

void createObjects() {
    Material matte = MatteMaterial(Color(0x0000FF));
    matte.setShading(GOURAUD);
    Material plastic = PlasticMaterial(Color(0x00FFFF));
    Material metal = MetalMaterial(Color(0x0000FF));
    Material glossy = GlossyMaterial(Color(0x0000FF));

    Geometry::Cube bigCube = Geometry::Cube(1);
    Geometry::Cube smallCube = Geometry::Cube(0.5);
    Geometry::Sphere sphere = Geometry::Sphere(1, 32);

    Mesh bcMesh = Mesh(bigCube, matte);
    bcMesh.setPosition(Vec3(2, 0, -3));
    scene.add(bcMesh);

    // Mesh scMesh = Mesh(smallCube, matte);
    // scMesh.setPosition(Vec3(2, -0.25, -2.25));
    // scene.add(scMesh);

    // Mesh sMesh = Mesh(sphere, matte);
    // sMesh.setPosition(Vec3(0, 2, -2));
    // scene.add(sMesh);
}

void createCameras() {
    float aspect = (float)WIDTH / (float)HEIGHT;
    float halfHeight = 2.5f;
    float halfWidth = halfHeight * aspect;
    Vec3 pos = Vec3(0, 0, 3);

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
    std::unique_ptr<Light> ambient_l = std::make_unique<AmbientLight>(Color(0x00ff00));
    scene.add(std::move(ambient_l));

    Vec3 pos1 = Vec3(5, 5, -1.5);
    std::unique_ptr<Light> light1 = std::make_unique<PointLight>(pos1, Color(0xff0000));
    scene.add(std::move(light1));

    Vec3 pos2 = Vec3(0, 5, 1.5);
    std::unique_ptr<Light> light2 = std::make_unique<PointLight>(pos2, Color(0xffffff));
    scene.add(std::move(light2));

    Vec3 pos3 = Vec3(-1.2, 1.5, 0);
    std::unique_ptr<Light> light3 = std::make_unique<PointLight>(pos3, Color(0xff0000), 14);
    scene.add(std::move(light3));
}

void update() {
    renderer.setWireframe(wireframe);
    renderer.setDepthShading(depthShading);
}

void display(uint32_t* framebuffer, SDL_Renderer* sdl_renderer, SDL_Texture* texture) {
    // clear to black
    std::fill(framebuffer, framebuffer + WIDTH * HEIGHT, 0);
    renderer.render(scene, scene.getCamera(cameraIndex));
    const Buffer<Fragment> &fragmentBuffer = renderer.fragmentBuffer();

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

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    uint32_t framebuffer[WIDTH * HEIGHT];

    SDL_Renderer* sdl_renderer = SDL_CreateRenderer(window.getSDLWindow(), NULL);
    SDL_Texture* texture = SDL_CreateTexture(
        sdl_renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        WIDTH, HEIGHT
    );

    renderer.setBackground(Color(0x00d3a0));

    // load OBJ model from command line argument
    if (argc < 2) {
        std::cerr << "Could not load 3D model.\n"
        << "Expected .obj file as argument but none provided.\n"
        << "Usage: " << argv[0] << " <path_to_obj_file>" << std::endl;
    } else {
        Material matte = MatteMaterial(Color(0x0000FF));
        std::optional<Mesh> objMesh = renderer.loadOBJModel(argv[1], matte);
        if (objMesh) {
            objMesh->setPosition(Vec3(0, -2, 0));
            objMesh->setRotationDegrees(Vec3(0, 90, 90));
            objMesh->setScale(Vec3(0.06, 0.06, 0.06));
            scene.add(*objMesh);
        } else {
            std::cerr << "Failed to load OBJ model from file: " << argv[1] << std::endl;
        }
    }

    createObjects();
    createCameras();
    createLights();

    SDL_Event event;
    bool running = true;
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
                if (event.key.key == SDLK_D) {
                    depthShading = !depthShading;
                }
                if (event.key.key == SDLK_W) {
                    wireframe = !wireframe;
                }
            }
            if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                float factor = (event.wheel.y < 0) ? 0.9 : 1.1;
                scene.getCamera(cameraIndex).zoom(factor);
            }
        }
        
        update();
        display(framebuffer, sdl_renderer, texture);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(sdl_renderer);
    SDL_Quit();
    return 0;
}