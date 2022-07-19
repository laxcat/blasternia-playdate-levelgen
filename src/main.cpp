#include "../engine/engine.h"
#include "../engine/MrManager.h"
#include "../engine/dev/print.h"
#include "LevGenQuad.h"
#include "LevelData.h"
#include <imgui.h>

Renderable * plane;
bgfx::ProgramHandle program;
Texture levelTexture;
LevelData levelData;

void resetCamera(Camera & c) {
    c.target      = {(float)levelData.w/2.f-7.f, (float)levelData.h/2.f, 0.f};
    c.distance    = 34;
    c.pitch       = 0;
    c.yaw         = 0;
    c.fov         = 0;
}

void printVbuf() {
    char buf[1024];
    int pen = 0;
    for (int i = 0; i < 4; ++i) {
        pen += snprintf(buf + pen, 1024-pen, "%d: ", i);
        pen += toString(((VertLevGen *)plane->buffer) + i, buf + pen, 1024-pen);
        pen += snprintf(buf + pen, 1024-pen, "\n");
    }
    print("%s", buf);
}

void setLevelSize(float w, float h) {
    float u = w / (float)LevelData::maxW;
    float v = h / (float)LevelData::maxH;

    VertLevGen * vb = LevGenQuad::vbufferForIndex(plane, 0);

    (vb+1)->u = u;
    (vb+2)->u = u;
    (vb+2)->v = v;
    (vb+3)->v = v;

    (vb+1)->x = w;
    (vb+2)->x = w;
    (vb+2)->y = h;
    (vb+3)->y = h;

    levelData.set(w, h);
    LevGenQuad::updateVBuffer(plane, 0);

    // print("set to %d %d (uv %f %f)\n", (int)w, (int)h, u, v);
}

void postInit() {
    program = mm.memSys.loadProgram(mm.assetsPath, "vs_levgen", "fs_levgen");

    // setup material/texture
    Material mat;
    setName(mat.name, "level_plane_mat");
    mat.baseColor = {1.f, 1.f, 1.f, 1.f};
    mat.roughness() = 1.f;
    mat.metallic() = 0.f;
    mat.specular() = 0.f;
    auto tex = levelTexture.createMutable(32, 32, 4, BGFX_SAMPLER_MAG_POINT);
    levelTexture.fillCheckered(0x000000ff, 0xffffffff);

    // setup plane
    plane = mm.rendSys.create(program, "level_plane");
    LevGenQuad::allocateBufferWithCount(plane, 1);
    LevGenQuad::create(plane, 0, {32.f, 32.f}, 0, true); // materialId = index in plane->materials
    // plane->meshes[0].model = glm::rotate(glm::mat4{1.f}, (float)M_PI_2, {1.f, 0.f, 0.f});
    // plane->meshes[0].model = glm::translate(plane->meshes[0].model, {16.f, 16.f, 0.f});
    plane->meshes[0].images.color = 0; // index of texture in plane->textures
    plane->textures.push_back(tex);
    plane->materials.push_back(mat);

    mm.rendSys.lights.dirDataDirAsEuler[0].x = -M_PI_2;
    mm.rendSys.lights.dirStrengthAmbientAt (0) = 1.f;
    mm.rendSys.lights.dirStrengthDiffuseAt (0) = 0.f;
    mm.rendSys.lights.dirStrengthSpecularAt(0) = 0.f;
    mm.rendSys.lights.dirStrengthOverallAt (0) = 1.f;

    mm.camera.projType = Camera::ProjType::Ortho;
    mm.camera.orthoResetFn = resetCamera;
    mm.camera.reset();

    // printVbuf();
}

void preShutdown() {
    levelTexture.destroy();
    bgfx::destroy(program);
}

void preEditor() {
    using namespace ImGui;

    if (CollapsingHeader("Level Gen", ImGuiTreeNodeFlags_DefaultOpen)) {

        DragInt2("Size", (int *)&levelData.w, 0.5, 3, 32);
        if (Button("Change Size")) {
            setLevelSize(levelData.w, levelData.h);
            mm.camera.reset();
        }

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
