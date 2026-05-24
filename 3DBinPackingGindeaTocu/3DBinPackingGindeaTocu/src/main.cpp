#include <pch.h>

import scene;
import model;
import loader;

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "3D Bin Packing");

    rlImGuiSetup(true);

    ProblemLoader& loader = ProblemLoader::get();
    Camera3D camera = {.position = {20, 20, 20},
                       .target = {0, 0, 0},
                       .up = {0, 1, 0},
                       .fovy = 70,
                       .projection = CAMERA_PERSPECTIVE};

    ImGuiIO& io = ImGui::GetIO();
    ImPlot::CreateContext();

    ProblemModel model = loader.loadFromFile();
    Scene& scene = Scene::get();
    scene.setCamera(&camera);
    scene.setModel(&model);

#ifdef __EMSCRIPTEN__
    struct MainLoopData {
        Scene* scene;
        Camera3D* camera;
    } mainLoopData{&scene, &camera};

    MaximizeWindow();

    emscripten_set_main_loop_arg(
        [](void* userData) {
            auto data = static_cast<MainLoopData*>(userData);
            auto& scene = *data->scene;
            auto& camera = *data->camera;

#else
    while (!WindowShouldClose()) {
#endif
            scene.handleInput();

            if (scene.isCursorLocked()) {
                UpdateCamera(&camera, CAMERA_FREE);
            }


            BeginDrawing();
            scene.prepareFrame();
            ClearBackground(RED);
            scene.drawBackground();

            BeginMode3D(camera);
            scene.drawScene();
            EndMode3D();

            scene.drawCrosshair();

            rlImGuiBegin();
            scene.drawUI();
            rlImGuiEnd();

            EndDrawing();
#ifdef __EMSCRIPTEN__
        },
        (void*)&mainLoopData, 0, true);
#else
    }

    ImPlot::DestroyContext();
    rlImGuiShutdown();
    CloseWindow();

    loader.saveToFile(model);
#endif
}
