#include "game.h"

#include "../engine/common.h"
#include "../engine/game_state.h"
#include "../engine/platform.h"
#include "ecs.h"
#include "factories.h"
#include "state.h"

#include <SDL3/SDL_log.h>
#include <cimgui.h>
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
  state->scale           = cf_v2(scale - 1, scale - 1);    // Scale up for resolution independence
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

  if (!validate_game_state(state)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "GameState validation failed in game_init");
    abort();
  }

  cf_app_set_canvas_size((int)state->canvas_size.x, (int)state->canvas_size.y);
  cf_app_set_size((int)state->canvas_size.x, (int)state->canvas_size.y);
  cf_app_center_window();
#ifdef DEBUG
  cf_app_init_imgui();
#endif
}

EXPORT bool game_update(void) {
  cf_arena_reset(&state->scratch_arena);

  update_systems(state);

  return true;
}

EXPORT void game_render(void) {
#ifdef DEBUG
  input_t  *input  = ecs_get(state->ecs, state->player_entity, state->components.input);
  weapon_t *weapon = ecs_get(state->ecs, state->player_entity, state->components.weapon);
  CF_V2    *pos    = ecs_get(state->ecs, state->player_entity, state->components.position);
  CF_V2    *vel    = ecs_get(state->ecs, state->player_entity, state->components.velocity);
  igBegin("Debug Info", NULL, 0);

  igText("FPS: %.2f", cf_app_get_framerate());

  igText("Player Position: (%.2f, %.2f)", pos->x, pos->y);
  igText("Player Velocity: (%.2f, %.2f)", vel->x, vel->y);

  igText("Input.UP: %d", input->up);
  igText("Input.DOWN: %d", input->down);
  igText("Input.LEFT: %d", input->left);
  igText("Input.RIGHT: %d", input->right);
  igText("Input.SHOOT: %d", input->shoot);

  igText("Weapon.Cooldown: %f", weapon->cooldown);
  igText("Weapon.TimeSinceShot: %f", weapon->time_since_shot);
  igEnd();
#endif
}

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
