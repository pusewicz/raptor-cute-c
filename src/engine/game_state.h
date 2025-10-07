#pragma once

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_audio.h>
#include <cute_color.h>
#include <cute_coroutine.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>

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
    int          score;
    int          lives;

    Entities   entities;
    Components components;
    Systems    systems;

    struct {
        CF_Coroutine spawner;
    } coroutines;

    struct {
        CF_Audio music;
        CF_Audio laser_shoot;
        CF_Audio explosion;
    } audio;

    struct {
        CF_Sprite life_icon;
    } sprites;

    bool debug_bounding_boxes;
} GameState;

extern GameState* g_state;

bool validate_game_state(void);
