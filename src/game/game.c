#include "game.h"

#include "../engine/common.h"
#include "../engine/game_state.h"
#include "../engine/platform.h"
#include "ecs.h"
#include "factories.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <stddef.h>
#include <stdio.h>

GameState *state = NULL;

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#define PERMANENT_ARENA_SIZE    MiB(64)
#define STAGE_ARENA_SIZE        MiB(64)
#define SCRATCH_ARENA_SIZE      MiB(64)
#define DEFAULT_ARENA_ALIGNMENT 16

#define CANVAS_WIDTH  180
#define CANVAS_HEIGHT 320

EXPORT void game_init(Platform *platform) {
  state = platform->allocate_memory(sizeof(GameState));

  state->display_id = cf_default_display();

  int scale_w = cf_display_width(state->display_id) / CANVAS_WIDTH;
  int scale_h = cf_display_height(state->display_id) / CANVAS_HEIGHT;
  int scale   = cf_min(scale_w, scale_h) - 1;

  int canvas_w = CANVAS_WIDTH * scale;
  int canvas_h = CANVAS_HEIGHT * scale;

  state->platform        = platform;
  state->canvas_size     = cf_v2(canvas_w, canvas_h);
  state->scale           = cf_v2(scale - 2, scale - 2);    // Scale up for resolution independence
  state->permanent_arena = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, PERMANENT_ARENA_SIZE);
  state->stage_arena     = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, STAGE_ARENA_SIZE);
  state->scratch_arena   = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, SCRATCH_ARENA_SIZE);
  state->ecs             = ecs_new(1024, NULL);

  // Initialize component and system IDs to 0
  state->components = (Components){0};
  state->systems    = (Systems){0};

  // Mark that we haven't registered yet
  state->components_registered = false;    // TODO: Do we need this?
  state->systems_registered    = false;    // TODO: Do we need this?

  register_components(state);
  register_systems(state);

  state->player_entity = make_player(state, 0.0f, 0.0f);

  cf_app_set_canvas_size((int)state->canvas_size.x, (int)state->canvas_size.y);
  cf_app_set_size((int)state->canvas_size.x, (int)state->canvas_size.y);
  cf_app_center_window();
}

EXPORT bool game_update(void) {
  cf_arena_reset(&state->scratch_arena);

  update_systems(state);

  return true;
}

EXPORT void game_render(void) {}

EXPORT void game_shutdown(void) {
  Platform *platform = state->platform;
  cf_destroy_arena(&state->scratch_arena);
  cf_destroy_arena(&state->stage_arena);
  cf_destroy_arena(&state->permanent_arena);
  ecs_free(state->ecs);
  platform->free_memory(state);
}

EXPORT void *game_state(void) { return state; }

EXPORT void game_hot_reload(void *game_state) {
  state = (GameState *)game_state;
  // Update system callbacks to new function addresses
  update_system_callbacks(state);
}
