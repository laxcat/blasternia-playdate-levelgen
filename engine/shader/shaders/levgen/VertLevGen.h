#pragma once
#include <stdio.h>

struct VertLevGen {
    float x;
    float y;
    float z;

    float nx;
    float ny;
    float nz;

    float u;
    float v;
};

inline int toString(VertLevGen const * v, char * out, size_t outSize) {
    int triedToWrite = snprintf(out, outSize,
        "xyz (%0.2f, %0.2f, %0.2f), norm (%0.2f, %0.2f, %0.2f), uv (%0.2f, %0.2f)",
        v->x, v->y, v->z,
        v->nx, v->ny, v->nz,
        v->u, v->v
    );
    return (triedToWrite < outSize) ? triedToWrite : outSize;
}
