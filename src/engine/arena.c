#include "arena.h"

#include <cute.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define ARENA_DEFAULT_ALIGNMENT 16

void arena_init(CF_Arena *arena, void *buffer, int size) {
  arena->ptr         = (char *)buffer;
  arena->end         = arena->ptr + size;
  arena->blocks      = NULL;
  arena->block_index = 0;
  arena->block_size  = size;
  arena->alignment   = ARENA_DEFAULT_ALIGNMENT;
}

void *arena_push(CF_Arena *arena, int size) { return cf_arena_alloc(arena, size); }
void  arena_reset(CF_Arena *arena) { cf_arena_reset(arena); }
