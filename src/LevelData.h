#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t byte_t;

#define LEVEL_DATA_MAX_W (32)
#define LEVEL_DATA_MAX_H (32)

enum LevelDataValue {
    LD_VALUE_UNKNOWN,
    LD_VALUE_OUT_OF_BOUNDS,
    LD_VALUE_BLOCK,
    LD_VALUE_START,
    LD_VALUE_END,
    LD_VALUE_CURRENT,
    LD_VALUE_OPEN,
    LD_VALUE_PATH,
    LD_VALUE_EXPANSION,
};
typedef enum LevelDataValue LevelDataValue;

typedef struct LevelData LevelData;
struct LevelData {
    int w;
    int h;
    byte_t data[LEVEL_DATA_MAX_W * LEVEL_DATA_MAX_H];
};

void LevelData_setSize(LevelData * ld, int w, int h);
void LevelData_setXY(LevelData * ld, int x, int y, LevelDataValue value);

#ifdef __cplusplus
};
#endif
