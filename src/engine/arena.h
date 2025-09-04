#pragma once

typedef struct CF_Arena CF_Arena;

void  arena_init(CF_Arena *arena, void *buffer, int size);
void *arena_push(CF_Arena *arena, int size);
void  arena_reset(CF_Arena *arena);
