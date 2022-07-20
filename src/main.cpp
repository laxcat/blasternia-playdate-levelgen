#include "../engine/engine.h"
#include "../engine/MrManager.h"
#include "../engine/dev/print.h"
#include "LevGenQuad.h"
#include "../../Playdate_proj/src/levelgen/LevelData.h"
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
        pen += snprintf(buf+pen, 1024-pen, "%d: ", i);
        pen += toString(((VertLevGen *)plane->buffer)+i, buf+pen, 1024-pen);
        pen += snprintf(buf+pen, 1024-pen, "\n");
    }
    print("%s", buf);
}

void updateDisplay() {
    // update texture
    int count = LEVEL_DATA_MAX_W*LEVEL_DATA_MAX_H;
    for (int i = 0; i < count; ++i) {
        uint32_t color =
            (levelData.data[i] == LD_VALUE_UNKNOWN)         ? 0xff00ffff :
            (levelData.data[i] == LD_VALUE_OUT_OF_BOUNDS)   ? 0x333333ff :
            (levelData.data[i] == LD_VALUE_BLOCK)           ? 0x000000ff :
            (levelData.data[i] == LD_VALUE_START)           ? 0x008800ff :
            (levelData.data[i] == LD_VALUE_END)             ? 0x0000ffff :
            (levelData.data[i] == LD_VALUE_CURRENT)         ? 0xcccc00ff :
            (levelData.data[i] == LD_VALUE_OPEN)            ? 0xffffffff :
            (levelData.data[i] == LD_VALUE_PATH)            ? 0x880000ff :
            (levelData.data[i] == LD_VALUE_EXPANSION)       ? 0x880033ff :
            0xff00ffff;
        if ((i % LEVEL_DATA_MAX_W + i / LEVEL_DATA_MAX_W) % 2) color -= 0x11;
        levelTexture.img.setPixel(i, color);
    }
    levelTexture.update();

    // update vbuffer
    float w = levelData.w;
    float h = levelData.h;
    float u = w / (float)LEVEL_DATA_MAX_W;
    float v = h / (float)LEVEL_DATA_MAX_H;

    VertLevGen * vb = LevGenQuad::vbufferForIndex(plane, 0);

    (vb+1)->u = u;
    (vb+2)->u = u;
    (vb+2)->v = v;
    (vb+3)->v = v;

    (vb+1)->x = w;
    (vb+2)->x = w;
    (vb+2)->y = h;
    (vb+3)->y = h;

    LevGenQuad::updateVBuffer(plane, 0);
}

void genAndUpdate() {
    LevelData_genSize(&levelData);
    LevelData_genStartEnd(&levelData);
    updateDisplay();
    mm.camera.reset();
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
    uint64_t texFlags = BGFX_SAMPLER_MAG_POINT|BGFX_SAMPLER_U_MIRROR|BGFX_SAMPLER_V_MIRROR;
    auto tex = levelTexture.createMutable(LEVEL_DATA_MAX_W, LEVEL_DATA_MAX_H, 4, texFlags);
    levelTexture.fillCheckered(0x000000ff, 0xffffffff);

    // setup plane
    plane = mm.rendSys.create(program, "level_plane");
    LevGenQuad::allocateBufferWithCount(plane, 1);
    LevGenQuad::create(plane, 0, {(float)LEVEL_DATA_MAX_W, (float)LEVEL_DATA_MAX_H}, 0, true);
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

    // setLevelSize(LEVEL_DATA_MAX_W, LEVEL_DATA_MAX_H);
    // printVbuf();
    LevelData_setLevel(&levelData, 1, 0);
    genAndUpdate();
}

void preShutdown() {
    levelTexture.destroy();
    bgfx::destroy(program);
}

void preEditor() {
    using namespace ImGui;

    if (CollapsingHeader("Level Gen", ImGuiTreeNodeFlags_DefaultOpen)) {

        if (InputInt("Seed", (int *)&levelData.seed)) {
            genAndUpdate();
        }

        if (DragInt("Level", (int *)&levelData.levelId, 0.5, 1, 500)) {
            genAndUpdate();
        }

        Separator();

        if (DragInt2("Size", (int *)&levelData.w, 0.5, 3, 32)) {
            LevelData_setSize(&levelData, levelData.w, levelData.h);
            updateDisplay();
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
