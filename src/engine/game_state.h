#pragma once

#include "../game/ecs.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <pico_ecs.h>

typedef struct Platform Platform;

/*
 * Game State
 *
 * Should be validated with validate_game_state() before use
 */
typedef struct GameState {
  Platform    *platform;
  CF_V2        canvas_size;
  float        scale;    // For resolution independence
  CF_Arena     permanent_arena;
  CF_Arena     stage_arena;
  CF_Arena     scratch_arena;
  ecs_t       *ecs;
  ecs_id_t     player_entity;
  ecs_id_t     enemy_spawner_entity;
  CF_DisplayID display_id;
  CF_Rnd       rnd;

  // Component and System IDs - persist across hot reloads
  Components components;
  Systems    systems;
} GameState;

extern GameState *g_state;
