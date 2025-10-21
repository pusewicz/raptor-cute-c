#include "scene.h"

#include "../engine/game_state.h"
#include "scene/gameplay.h"
#include "scene/intro.h"

static Scene current_scene;

void scene_init(SceneType type) {
    g_state->current_scene = type;

    switch (type) {
        case SCENE_INTRO:
            current_scene.init    = intro_init;
            current_scene.update  = intro_update;
            current_scene.render  = intro_render;
            current_scene.cleanup = intro_cleanup;
            break;
        case SCENE_GAMEPLAY:
            current_scene.init    = gameplay_init;
            current_scene.update  = gameplay_update;
            current_scene.render  = gameplay_render;
            current_scene.cleanup = gameplay_cleanup;
            break;
    }

    if (current_scene.init) { current_scene.init(); }
}

void scene_switch(SceneType type) {
    if (current_scene.cleanup) { current_scene.cleanup(); }
    scene_init(type);
}

Scene scene_get_current(void) { return current_scene; }
