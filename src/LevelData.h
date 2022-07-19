#pragma once
#include "../engine/common/types.h"

class LevelData {
public:
    static constexpr int maxW = 32;
    static constexpr int maxH = 32;
    int w = maxW;
    int h = maxH;

    byte_t data[maxW * maxH];

    Image * img;

    void set(int w, int h) {
        this->w = w;
        this->h = h;
    }

    void updateDisplay() {
        if (!img) return;
    }
};
