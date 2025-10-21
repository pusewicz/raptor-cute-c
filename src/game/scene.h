#pragma once

#include <stdbool.h>

// Scene types
typedef enum SceneType {
    SCENE_INTRO,
    SCENE_GAMEPLAY,
} SceneType;

// Scene interface
typedef struct Scene {
    void (*init)(void);
    bool (*update)(void);
    void (*render)(void);
    void (*cleanup)(void);
} Scene;

// Scene management functions
void  scene_init(SceneType type);
void  scene_switch(SceneType type);
Scene scene_get_current(void);
