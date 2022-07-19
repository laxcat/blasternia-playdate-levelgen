#include "LevelData.h"
#include <stdio.h>

void LevelData_setSize(LevelData * ld, int w, int h) {
    ld->w = w;
    ld->h = h;

    for (int x = 0; x < LEVEL_DATA_MAX_W; ++x) {
        for (int y = 0; y < LEVEL_DATA_MAX_H; ++y) {
            int value = (x < w && y < h) ? LD_VALUE_OPEN : LD_VALUE_OUT_OF_BOUNDS;
            LevelData_setXY(ld, x, y, value);
        }
    }
}

void LevelData_setXY(LevelData * ld, int x, int y, LevelDataValue value) {
    // printf("setting xy (%d, %d) to %d\n", x, y, value);
    ld->data[x + y * LEVEL_DATA_MAX_W] = value;
}
