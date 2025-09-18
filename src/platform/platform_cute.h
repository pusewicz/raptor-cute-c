#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Platform Platform;

typedef void (*GameInitFunction)(Platform *platform);
typedef bool (*GameUpdateFunction)(void);
typedef void *(*GameRenderFunction)(void);
typedef void (*GameShutdownFunction)(void);
typedef void *(*GameStateFunction)(void);
typedef void (*GameHotReloadFunction)(void *game_state);

typedef struct GameLibrary {
  void       *library;
  const char *path;

  GameInitFunction      init;
  GameUpdateFunction    update;
  GameRenderFunction    render;
  GameShutdownFunction  shutdown;
  GameStateFunction     state;
  GameHotReloadFunction hot_reload;

  bool ok;
} GameLibrary;

void platform_init(const char *argv0);
void platform_shutdown(void);

void *platform_allocate_memory(size_t size);
void  platform_free_memory(void *p);

void platform_begin_frame(void);
void platform_end_frame(void);

GameLibrary platform_load_game_library(void);
void        platform_unload_game_library(GameLibrary *game_library);

uint64_t platform_get_performance_counter(void);
uint64_t platform_get_performance_frequency(void);
