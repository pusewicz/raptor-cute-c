#pragma once

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_math.h>
#include <cute_rnd.h>

#include "../game/ecs.h"
#include "pico_ecs.h"

typedef struct Platform Platform;

/*
 * Game State
 *
 * Should be validated with validate_game_state() before use
 */
typedef struct GameState {
    Platform*    platform;
    CF_V2        canvas_size;
    float        scale;  // For resolution independence
    CF_Arena     permanent_arena;
    CF_Arena     stage_arena;
    CF_Arena     scratch_arena;
    CF_DisplayID display_id;
    CF_Rnd       rnd;
    ecs_t*       ecs;

    // ECS IDs - persist across hot reloads
    Entities   entities;
    Components components;
    Systems    systems;

    bool debug_bounding_boxes;
} GameState;

extern GameState* g_state;

bool validate_game_state(void);
