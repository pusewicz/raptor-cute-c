#include "../engine/common.h"
#include "../engine/game_input.h"
#include "../engine/game_memory.h"
#include "../engine/game_state.h"

#include <cute.h>

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#define PERMANENT_ARENA_SIZE MiB(64)
#define STAGE_ARENA_SIZE     MiB(64)
#define SCRATCH_ARENA_SIZE   MiB(64)

EXPORT void game_init(GameMemory *memory);
EXPORT bool game_update(GameMemory *memory, GameInput *input);
EXPORT void game_render(GameMemory *memory);
EXPORT void game_shutdown(GameMemory *memory);

EXPORT void game_init(GameMemory *memory) {
  GameState *state = (GameState *)memory->bytes;

  state->bg_color = cf_make_color_rgba(100, 149, 237, 255);

  state->permanent_arena = (CF_Arena *)(memory->bytes + sizeof(GameState));
  arena_init(state->permanent_arena, state->permanent_arena + sizeof(CF_Arena), PERMANENT_ARENA_SIZE);

  state->stage_arena = (CF_Arena *)state->permanent_arena->end;
  arena_init(state->stage_arena, state->stage_arena + sizeof(CF_Arena), STAGE_ARENA_SIZE);

  state->scratch_arena = (CF_Arena *)state->stage_arena->end;
  arena_init(state->scratch_arena, state->scratch_arena + sizeof(CF_Arena), SCRATCH_ARENA_SIZE);
}

EXPORT bool game_update(GameMemory *memory, GameInput *input) {
  (void)memory;
  (void)input;

  return true;
}

EXPORT void game_render(GameMemory *memory) { (void)memory;
  float fps = cf_app_get_smoothed_framerate();
  char fps_text[32];
  snprintf(fps_text, sizeof(fps_text), "FPS: %.2f", fps);
  cf_draw_text(fps_text, cf_v2(10, 30), -1);
}

EXPORT void game_shutdown(GameMemory *memory) { (void)memory; }
