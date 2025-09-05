#pragma once

#include <cute_alloc.h>
#include <cute_color.h>
#include <pico_ecs.h>

typedef struct Platform Platform;

typedef struct GameState {
  Platform *platform;
  CF_Color  bg_color;
  CF_Arena  permanent_arena;
  CF_Arena  stage_arena;
  CF_Arena  scratch_arena;
  ecs_t    *ecs;
  ecs_id_t  player_entity;
} GameState;

extern GameState *state;
