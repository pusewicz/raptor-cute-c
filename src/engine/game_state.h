#pragma once

#include <cute_alloc.h>
#include <cute_color.h>

typedef struct GameState {
  CF_Color bg_color;

  CF_Arena permanent_arena;
  CF_Arena stage_arena;
  CF_Arena scratch_arena;
} GameState;

void arena_init(CF_Arena *arena, void *buffer, int size);
