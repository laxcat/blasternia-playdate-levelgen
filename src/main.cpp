#include "../engine/engine.h"
#include "../engine/dev/print.h"
#include "../engine/MrManager.h"
#include "../engine/primitives/Quad.h"

void postInit() {
    print("postInit\n");

    auto r = mm.rendSys.create(mm.rendSys.gltfProgram, "level_plane");
    Quad::allocateBufferWithCount(r, 1);
    Material mat;
    setName(mat.name, "level_plane_mat");
    mat.baseColor = {1.f, 1.f, 1.f, 1.f};
    r->materials.push_back(mat);
    Quad::create(r, 0, {10.f, 10.f}, 0);
    r->meshes[0].model = glm::rotate(glm::mat4{1.f}, (float)M_PI_2, {1.f, 0.f, 0.f});

}

int main() {
    return main_desktop({
        // .preInit = preInit,
        .postInit = postInit,
        // .preEditor = preEditor,
        // .postEditor = postEditor,
        // .preDraw = preDraw,
        // .postDraw = postDraw,
        // .preResize = preResize,
        // .postResize = postResize,
        // .preShutdown = preShutdown,
        // .postShutdown = postShutdown,
        // .transparentFramebuffer = true,
    });
}


// void preInit()      { print("preInit\n"); }
// void preEditor()    { print("preEditor\n"); }
// void postEditor()   { print("postEditor\n"); }
// void preDraw()      { print("preDraw\n"); }
// void postDraw()     { print("postDraw\n"); }
// void preResize()    { print("preResize\n"); }
// void postResize()   { print("postResize\n"); }
// void preShutdown()  { print("preShutdown\n"); }
// void postShutdown() { print("postShutdown\n"); }
