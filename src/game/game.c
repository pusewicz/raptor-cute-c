#include "game.h"

#include "../engine/common.h"
#include "../engine/game_state.h"
#include "../engine/platform.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <stddef.h>
#include <stdio.h>

GameState *state = nullptr;

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#define PERMANENT_ARENA_SIZE    MiB(64)
#define STAGE_ARENA_SIZE        MiB(64)
#define SCRATCH_ARENA_SIZE      MiB(64)
#define DEFAULT_ARENA_ALIGNMENT 16

EXPORT void game_init(Platform *platform) {
  state = platform->allocate_memory(sizeof(GameState));

  state->platform        = platform;
  state->bg_color        = cf_make_color_rgba(100, 149, 237, 255);
  state->permanent_arena = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, PERMANENT_ARENA_SIZE);
  state->stage_arena     = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, STAGE_ARENA_SIZE);
  state->scratch_arena   = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, SCRATCH_ARENA_SIZE);
}

EXPORT bool game_update(void) {
  cf_arena_reset(&state->scratch_arena);

  return true;
}

EXPORT void game_render(void) {
  float fps = cf_app_get_smoothed_framerate();
  char  fps_text[32];
  snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", fps);
  cf_draw_text(fps_text, cf_v2(10, 30), -1);
}

EXPORT void game_shutdown(void) {
  Platform *platform = state->platform;
  cf_destroy_arena(&state->scratch_arena);
  cf_destroy_arena(&state->stage_arena);
  cf_destroy_arena(&state->permanent_arena);
  platform->free_memory(state);
}

EXPORT void *game_state(void) { return state; }

EXPORT void game_hot_reload(void *game_state) { state = (GameState *)game_state; }
