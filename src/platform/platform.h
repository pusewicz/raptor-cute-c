#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct GameMemory          GameMemory;
typedef struct GameInput           GameInput;
typedef struct RenderCommandBuffer RenderCommandBuffer;

typedef void (*GameInitFunction)(GameMemory *memory);
typedef bool (*GameUpdateFunction)(GameMemory *memory, GameInput *input);
typedef void *(*GameRenderFunction)(GameMemory *memory);
typedef void (*GameShutdownFunction)(GameMemory *memory);

typedef struct GameLibrary {
  void       *library;
  const char *library_name;

  GameInitFunction     init;
  GameUpdateFunction   update;
  GameRenderFunction   render;
  GameShutdownFunction shutdown;
} GameLibrary;

void platform_init(const char *argv0);
void platform_shutdown(void);

void *platform_allocate_memory(int size);
void  platform_free_memory(void *p);

void platform_update_input(GameInput *input);

void platform_begin_frame(void);
void platform_end_frame(void);

GameLibrary platform_open_game_library(void);
void        platform_reload_game_library(GameLibrary *library);
void        platform_close_game_library(GameLibrary *library);

uint64_t platform_get_performance_counter(void);
uint64_t platform_get_performance_frequency(void);
