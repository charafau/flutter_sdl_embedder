#include "embedder.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <cassert>
#include <chrono>
#include <iostream>
#include <stdio.h>
#include <string_view>
#include <codecvt>

#include <cstdint>
#include <numeric>

static double g_pixelRatio = 1.0;
static const size_t kInitialWindowWidth = 800;
static const size_t kInitialWindowHeight = 600;

constexpr float scaleFactor = 1.0f;

void printUsage()
{
    std::cout << "usage: flutter_glfw <path to project> <path to icudtl.dat>"
              << std::endl;
}

void updateSize(FlutterEngine engine, size_t width, size_t height, float pixelRatio, bool maximized)
{
    // Round up the physical window size to a multiple of the pixel ratio
    width = std::ceil(width / pixelRatio) * pixelRatio;
    height = std::ceil(height / pixelRatio) * pixelRatio;
    FlutterWindowMetricsEvent event = {0};
    event.struct_size = sizeof(event);
    event.width = width * scaleFactor;
    event.height = height * scaleFactor;
    event.pixel_ratio = pixelRatio * scaleFactor;

    FlutterEngineSendWindowMetricsEvent(engine, &event);
}

void updatePointer(FlutterEngine engine, FlutterPointerPhase phase, double x, double y, size_t timestamp)
{
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;
    event.x = x * scaleFactor;
    event.y = y * scaleFactor;
    event.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    FlutterEngineSendPointerEvent(reinterpret_cast<FlutterEngine>(engine), &event, 1);
}

FlutterEngine RunFlutter(SDL_Window *window,
                         SDL_GLContext &context,
                         const std::string &project_path,
                         const std::string &icudtl_path)
{
    SDL_SetWindowData(window, "GL", context);

    FlutterRendererConfig config = {};
    config.type = kOpenGL;
    config.open_gl.struct_size = sizeof(config.open_gl);
    config.open_gl.make_current = [](void *userdata) -> bool
    {
        auto window = (SDL_Window *)userdata;
        SDL_GL_MakeCurrent(window, SDL_GetWindowData(window, "GL"));
        return true;
    };
    config.open_gl.clear_current = [](void *) -> bool
    {
        SDL_GL_MakeCurrent(nullptr, nullptr);
        return true;
    };
    config.open_gl.present = [](void *userdata) -> bool
    {
        SDL_GL_SwapWindow((SDL_Window *)userdata);
        SDL_Event event = {0};
        event.type = SDL_USEREVENT;

        SDL_PushEvent(&event);
        return true;
    };
    config.open_gl.fbo_callback = [](void *) -> uint32_t
    {
        return 0; // FBO0
    };

    std::string assets_path = project_path + "/build/flutter_assets";
    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = assets_path.c_str(),
        .icu_data_path =
            icudtl_path.c_str(), // Find this in your bin/cache directory.
    };
    FlutterEngine engine = nullptr;
    FlutterEngineResult result =
        FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, // renderer
                         &args, window, &engine);
    assert(result == kSuccess && engine != nullptr);

    //   auto [w, h, dpi] = roundWindowSize(window);
    updateSize(engine, kInitialWindowWidth, kInitialWindowHeight, 1, SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED);

    return engine;
}

int main(int argc, char *argv[])
{

    if (argc != 3)
    {
        printUsage();
        return 1;
    }
    std::string project_path = argv[1];
    std::string icudtl_path = argv[2];

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {
        printf("error initializin SDL: %s\n", SDL_GetError());
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Window *window = SDL_CreateWindow(
        "SDL2Test",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        kInitialWindowWidth,
        kInitialWindowHeight,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_GLContext maincontext;
    int framebuffer_width, framebuffer_height;
    SDL_GL_GetDrawableSize(window, &framebuffer_width, &framebuffer_height);
    SDL_GL_SetSwapInterval(1);

    g_pixelRatio = framebuffer_width / kInitialWindowWidth;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, SDL_ALPHA_OPAQUE);
    SDL_Event input;
    bool quit = false;
    int r = 0, g = 255, b = 0;
    maincontext = SDL_GL_CreateContext(window);

    bool mouseDown = false;
    int mouseId = 0;
    int lastMouseX, lastMouseY;

    FlutterEngine engine = RunFlutter(window, maincontext, project_path, icudtl_path);
    while (!quit) //Same as while(quit == false)
    {

        //---Event polling method---
        //It runs until the number of events to be polled gets to zero
        while (SDL_PollEvent(&input) > 0)
        {

            //If the user did something which should
            //result in quitting of the game then...
            if (input.type == SDL_QUIT)
            {
                //...go out of the while loop
                quit = true;
            }





            // Mouse button support
            if (input.type == SDL_MOUSEBUTTONDOWN)
            {
                if (!mouseDown)
                {
                    mouseDown = true;
                    mouseId = input.button.which;
                    lastMouseX = input.button.x;
                    lastMouseY = input.button.y;
                    updatePointer(engine, FlutterPointerPhase::kDown, input.button.x, input.button.y, input.button.timestamp);
                }
                // printf("mouse down");
            }

            if (input.type == SDL_MOUSEBUTTONUP)
            {
                if (mouseDown && mouseId == input.button.which)
                {
                    mouseDown = false;
                    lastMouseX = input.button.x;
                    lastMouseY = input.button.y;
                    updatePointer(engine, FlutterPointerPhase::kUp, input.button.x, input.button.y, input.button.timestamp);
                }
                // printf("mouse up");
            }

            if (input.type == SDL_MOUSEMOTION)
            {
                if (mouseDown && mouseId == input.button.which)
                {
                    lastMouseX = input.button.x;
                    lastMouseY = input.button.y;
                    updatePointer(engine, FlutterPointerPhase::kMove, input.button.x, input.button.y, input.button.timestamp);
                }
                // printf("mouse up");
            }
        }

        // SDL_SetRenderDrawColor(renderer, r, g, b, 255);
        // SDL_RenderClear(renderer);
        // SDL_RenderPresent(renderer);
        __FlutterEngineFlushPendingTasksNow();
    }
    // SDL_RenderClear(renderer);
    // SDL_RenderPresent(renderer);

    // SDL_Delay(3000);
    SDL_DestroyRenderer(renderer);
    SDL_GL_DeleteContext(maincontext);

    SDL_DestroyWindow(window);
    SDL_Quit();

    return EXIT_SUCCESS;
}