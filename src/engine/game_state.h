#pragma once

#include "../game/ecs.h"

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wsign-conversion"
  #pragma GCC diagnostic ignored "-Wfloat-conversion"
  #pragma GCC diagnostic ignored "-Wshorten-64-to-32"
  #pragma GCC diagnostic ignored "-Wimplicit-int-conversion"
#endif

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_math.h>
#include <pico_ecs.h>

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif

typedef struct Platform Platform;

typedef struct GameState {
  Platform    *platform;
  CF_V2        canvas_size;
  CF_V2        scale;    // For resolution independence
  CF_Arena     permanent_arena;
  CF_Arena     stage_arena;
  CF_Arena     scratch_arena;
  ecs_t       *ecs;
  ecs_id_t     player_entity;
  CF_DisplayID display_id;

  // Component and System IDs - persist across hot reloads
  Components components;
  Systems    systems;

  // Track if components/systems have been registered
  bool components_registered;
  bool systems_registered;
} GameState;

extern GameState *state;
