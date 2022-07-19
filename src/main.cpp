#include "../engine/engine.h"
#include "../engine/MrManager.h"
#include "../engine/dev/print.h"
#include "../engine/primitives/Quad.h"
#include <imgui.h>

Renderable * plane;
bgfx::ProgramHandle program;
Texture levelTexture;

// constexpr size_t W = 32;
// constexpr size_t H = 32;
// constexpr size_t C = 4;
// constexpr size_t levelMaxSize = W*H;

void resetCamera(Camera & c) {
    c.target      = {8.f, 16.f, 0.f};
    c.distance    = 34;
    c.pitch       = 0;
    c.yaw         = 0;
    c.fov         = 0;
}

void postInit() {
    program = mm.memSys.loadProgram(mm.assetsPath, "vs_levgen", "fs_levgen");

    // setup plane
    plane = mm.rendSys.create(program, "level_plane");
    Quad::allocateBufferWithCount(plane, 1);

    Material mat;
    setName(mat.name, "level_plane_mat");
    mat.baseColor = {1.f, 1.f, 1.f, 1.f};
    mat.roughness() = 1.f;
    mat.metallic() = 0.f;
    mat.specular() = 0.f;

    auto handle = levelTexture.createMutable(32, 32, 4, BGFX_SAMPLER_MAG_POINT);
    plane->textures.push_back(handle);
    plane->materials.push_back(mat);
    levelTexture.fillCheckered(0x000000ff, 0xffffffff);

    Quad::create(plane, 0, {32.f, 32.f}, 0);
    // plane->meshes[0].model = glm::rotate(glm::mat4{1.f}, (float)M_PI_2, {1.f, 0.f, 0.f});
    plane->meshes[0].model = glm::translate(plane->meshes[0].model, {16.f, 16.f, 0.f});
    plane->meshes[0].images.color = 0;

    mm.rendSys.lights.dirDataDirAsEuler[0].x = -M_PI_2;
    mm.rendSys.lights.dirStrengthAmbientAt (0) = 1.f;
    mm.rendSys.lights.dirStrengthDiffuseAt (0) = 0.f;
    mm.rendSys.lights.dirStrengthSpecularAt(0) = 0.f;
    mm.rendSys.lights.dirStrengthOverallAt (0) = 1.f;

    mm.camera.projType = Camera::ProjType::Ortho;
    mm.camera.orthoResetFn = resetCamera;
    mm.camera.reset();
}

void preShutdown() {
    levelTexture.destroy();
    bgfx::destroy(program);
}

void preEditor() {
    using namespace ImGui;

    if (CollapsingHeader("Level Gen", ImGuiTreeNodeFlags_DefaultOpen)) {

        Separator();
        TextUnformatted("Utility");
        PushItemWidth(200);
        ColorEdit4("Base Color", (float *)&plane->materials[0].baseColor, ImGuiColorEditFlags_DisplayHex);
        static float a[4] = {0.f, 0.f, 0.f, 1.f};
        static float b[4] = {1.f, 1.f, 1.f, 1.f};
        ColorEdit4("A", (float *)&a, ImGuiColorEditFlags_DisplayHex);
        ColorEdit4("B", (float *)&b, ImGuiColorEditFlags_DisplayHex);
        PopItemWidth();
        if (Button("Fill Solid A")) {
            levelTexture.fill((float *)&a);
        }
        SameLine();
        if (Button("Fill Checkered")) {
            levelTexture.fillCheckered((float *)&a, (float *)&b);
        }

        Dummy(ImVec2(0.0f, 20.0f));
    }
}

int main() {
    return main_desktop({
        .postInit = postInit,
        .preShutdown = preShutdown,
        .preEditor = preEditor,
    });
}
