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
bool showCurrent = true;

void resetCamera(Camera & c) {
    c.target      = {(float)levelData.w/2.f-7.f, (float)levelData.h/2.f, 0.f};
    c.distance    = LEVEL_DATA_MAX_H + 2.f;
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

void showDistanceData() {
    uint16_t count = LEVEL_DATA_MAX_W * LEVEL_DATA_MAX_H;
    for (uint16_t i = 0; i < count; ++i) {
        if (i % LEVEL_DATA_MAX_W >= levelData.w || i / LEVEL_DATA_MAX_W >= levelData.h) continue;
        byte_t v = (byte_t)(levelData.distanceData[i] * 255.f);
        uint32_t color =
            v << 24 |
            v << 16 |
            v <<  8 |
            0xff;
        // print("(%2d,%2d) color: 0x%08x\n", i % LEVEL_DATA_MAX_W, i / LEVEL_DATA_MAX_W, color);
        levelTexture.img.setPixel(i, color);
    }
    levelTexture.update();
}

void updateDisplay() {
    // update texture
    uint16_t count = LEVEL_DATA_MAX_W * LEVEL_DATA_MAX_H;
    for (uint16_t i = 0; i < count; ++i) {
        uint32_t color =
            (levelData.data[i] == LD_VALUE_UNKNOWN)         ? 0xff00ffff :
            (levelData.data[i] == LD_VALUE_OUT_OF_BOUNDS)   ? 0x333333ff :
            (levelData.data[i] == LD_VALUE_BLOCK)           ? 0x000000ff :
            (levelData.data[i] == LD_VALUE_START)           ? 0x008800ff :
            (levelData.data[i] == LD_VALUE_END)             ? 0x0000ffff :
            (levelData.data[i] == LD_VALUE_OPEN)            ? 0xffffffff :
            (levelData.data[i] == LD_VALUE_PATH)            ? 0x880000ff :
            (levelData.data[i] == LD_VALUE_EXPANSION)       ? 0x880033ff :
            0xff00ffff;
        if ((i % LEVEL_DATA_MAX_W + i / LEVEL_DATA_MAX_W) % 2) color -= 0x11;
        levelTexture.img.setPixel(i, color);

        if (showCurrent && levelData.currentIndex == i) {
            levelTexture.img.setPixel(i, 0xff9900ff);
        }
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
    LevelData_genPath(&levelData, NULL);
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

        Dummy(ImVec2(0.0f, 20.0f));

        if (Checkbox("Show Current & Target", &showCurrent)) {
            updateDisplay();
        }

        if (InputInt("Seed", (int *)&levelData.seed)) {
            genAndUpdate();
        }
        if (DragInt("Level", (int *)&levelData.levelId, 0.5, 1, 500)) {
            genAndUpdate();
        }

        Dummy(ImVec2(0.0f, 20.0f));

        if (Button("Update Display")) {
            updateDisplay();
            mm.camera.reset();
        }

        Dummy(ImVec2(0.0f, 20.0f));
        Separator();
        Dummy(ImVec2(0.0f, 20.0f));

        PushItemWidth(200);
        bool widthChanged = DragInt("W", (int *)&levelData.w, 0.5, LEVEL_DATA_MIN_W, LEVEL_DATA_MAX_W);
        SameLine();
        bool heightChanged = DragInt("H", (int *)&levelData.h, 0.5, LEVEL_DATA_MIN_H, LEVEL_DATA_MAX_H);
        PopItemWidth();
        if (widthChanged || heightChanged) {
            LevelData_setSize(&levelData, levelData.w, levelData.h);
            updateDisplay();
            mm.camera.reset();
        }
        if (Button("Gen From Current Size")) {
            LevelData_genStartEnd(&levelData);
            updateDisplay();
            mm.camera.reset();
        }

        Dummy(ImVec2(0.0f, 20.0f));

        if (CollapsingHeader("Utility", ImGuiTreeNodeFlags_DefaultOpen)) {
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
            if (Button("Fill w Dist Data")) {
                showDistanceData();
            }
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
