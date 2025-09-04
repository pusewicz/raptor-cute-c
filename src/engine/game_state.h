#pragma once

#include <cute_alloc.h>
#include <cute_color.h>

typedef struct Platform Platform;

typedef struct GameState {
  Platform *platform;
  CF_Color  bg_color;
  CF_Arena  permanent_arena;
  CF_Arena  stage_arena;
  CF_Arena  scratch_arena;
} GameState;

extern GameState *state;
