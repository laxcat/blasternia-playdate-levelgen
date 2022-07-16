#include "../engine/engine.h"
#include "../engine/dev/print.h"
#include "../engine/MrManager.h"
#include "../engine/primitives/Quad.h"
#include <imgui.h>

Renderable * plane;
bgfx::ProgramHandle program;
Texture levelTexture;


byte_t levelData[32*32*4];

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

    size_t levelDataSize = sizeof(levelData);
    print("SIZE OF LEVEL DATA %zu\n", levelDataSize);
    for (size_t i = 0; i < levelDataSize; i += 4) {
        bool white = i % 8;
        if ((i / (32*4))% 2) white = !white;
        if (white) {
            levelData[i+0] = 0x33;
            levelData[i+1] = 0x33;
            levelData[i+2] = 0x33;
            levelData[i+3] = 0xff;
        }
        else {
            levelData[i+0] = 0x00;
            levelData[i+1] = 0x00;
            levelData[i+2] = 0x00;
            levelData[i+3] = 0xff;
        }
    }

    levelTexture.allocImageAndCreate(32, 32, 4, levelData);
    auto handle = levelTexture.createTexture2D(BGFX_SAMPLER_MAG_POINT);
    plane->textures.push_back(handle);
    plane->materials.push_back(mat);

    Quad::create(plane, 0, {10.f, 10.f}, 0);
    plane->meshes[0].model = glm::rotate(glm::mat4{1.f}, (float)M_PI_2, {1.f, 0.f, 0.f});
    plane->meshes[0].images.color = 0;

    mm.rendSys.lights.dirDataDirAsEuler[0].x = -M_PI_2;
    mm.rendSys.lights.dirStrengthAmbientAt (0) = 1.f;
    mm.rendSys.lights.dirStrengthDiffuseAt (0) = 0.f;
    mm.rendSys.lights.dirStrengthSpecularAt(0) = 0.f;
    mm.rendSys.lights.dirStrengthOverallAt (0) = 1.f;
}

void preShutdown() {
    levelTexture.destroy();
}

void preEditor() {
    if (ImGui::CollapsingHeader("Level Gen", ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::ColorEdit4("Base Color", (float *)&plane->materials[0].baseColor, ImGuiColorEditFlags_DisplayHex);
    }

}

int main() {
    return main_desktop({
        .postInit = postInit,
        .preShutdown = preShutdown,
        .preEditor = preEditor,
    });
}
