#pragma once

#include <cute_defines.h>

#include "../engine/platform.h"

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

#define INIT_ENTITY_STORAGE(type, field, max)                                                \
    g_state->field            = cf_arena_alloc(&g_state->stage_arena, (max) * sizeof(type)); \
    g_state->field##_count    = 0;                                                           \
    g_state->field##_capacity = (max)

constexpr int PERMANENT_ARENA_SIZE         = CF_MB * 64;
constexpr int STAGE_ARENA_SIZE             = CF_MB * 64;
constexpr int SCRATCH_ARENA_SIZE           = CF_MB * 64;
constexpr int DEFAULT_ARENA_ALIGNMENT      = 16;

constexpr int CANVAS_WIDTH                 = 180;
constexpr int CANVAS_HEIGHT                = 320;
constexpr int CANVAS_SCALE                 = 3;

constexpr int MAX_PLAYER_BULLETS           = 32;
constexpr int MAX_ENEMIES                  = 128;
constexpr int MAX_ENEMY_BULLETS            = 128;
constexpr int MAX_HIT_PARTICLES            = 240;
constexpr int MAX_EXPLOSION_PARTICLES      = 320;    // More particles for colorful explosions
constexpr int MAX_EXPLOSIONS               = 32;
constexpr int MAX_STAR_PARTICLES           = 4 * 4;  // 4 stars per 4 layers
constexpr int MAX_FLOATING_SCORES          = 16;

constexpr float WAVE_ANNOUNCEMENT_DURATION = 2.0f;

typedef struct Platform Platform;

EXPORT void  game_init(Platform* platform);
EXPORT bool  game_update(void);
EXPORT void  game_render(void);
EXPORT void  game_shutdown(void);
EXPORT void* game_state(void);
EXPORT void  game_hot_reload(void* game_state);
