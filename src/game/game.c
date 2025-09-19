#include "game.h"

#include "../engine/common.h"
#include "../engine/game_state.h"
#include "cute_time.h"
#include "ecs.h"
#include "factories.h"
#include "pico_ecs.h"
#include "state.h"

#include <SDL3/SDL_log.h>
#include <_abort.h>
#include <cimgui.h>
#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

GameState *g_state = NULL;

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
  g_state = platform->allocate_memory(sizeof(GameState));

  const int scale    = 3;
  const int canvas_w = CANVAS_WIDTH * scale;
  const int canvas_h = CANVAS_HEIGHT * scale;

  g_state->display_id      = cf_default_display();
  g_state->platform        = platform;
  g_state->canvas_size     = cf_v2(canvas_w, canvas_h);
  g_state->scale           = scale;
  g_state->permanent_arena = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, PERMANENT_ARENA_SIZE);
  g_state->stage_arena     = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, STAGE_ARENA_SIZE);
  g_state->scratch_arena   = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, SCRATCH_ARENA_SIZE);
  g_state->ecs             = ecs_new(1024, NULL);
  g_state->rnd             = cf_rnd_seed((uint32_t)time(NULL));

  // Initialize component and system IDs to 0
  g_state->components = (Components){0};
  g_state->systems    = (Systems){0};

  register_components();
  register_systems();

  g_state->player_entity        = make_player(0.0f, 0.0f);
  g_state->enemy_spawner_entity = make_enemy_spawner();

  if (!validate_game_state()) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "GameState validation failed in game_init");
    abort();
  }

  cf_app_set_canvas_size((int)g_state->canvas_size.x, (int)g_state->canvas_size.y);
  cf_app_set_size((int)g_state->canvas_size.x, (int)g_state->canvas_size.y);
  cf_app_center_window();
#ifdef DEBUG
  cf_app_init_imgui();
#endif
}

EXPORT bool game_update(void) {
  cf_arena_reset(&g_state->scratch_arena);

  ecs_update_system(g_state->ecs, g_state->systems.input, CF_DELTA_TIME);
  ecs_update_system(g_state->ecs, g_state->systems.movement, CF_DELTA_TIME);
  ecs_update_system(g_state->ecs, g_state->systems.weapon, CF_DELTA_TIME);
  ecs_update_system(g_state->ecs, g_state->systems.enemy_spawn, CF_DELTA_TIME);
  ecs_update_system(g_state->ecs, g_state->systems.collision, CF_DELTA_TIME);

  return true;
}

static void game_render_debug(void) {
  InputComponent  *input  = ecs_get(g_state->ecs, g_state->player_entity, g_state->components.input);
  WeaponComponent *weapon = ecs_get(g_state->ecs, g_state->player_entity, g_state->components.weapon);
  CF_V2           *pos    = ecs_get(g_state->ecs, g_state->player_entity, g_state->components.position);
  CF_V2           *vel    = ecs_get(g_state->ecs, g_state->player_entity, g_state->components.velocity);

  EnemySpawnComponent *spawn = ecs_get(g_state->ecs, g_state->enemy_spawner_entity, g_state->components.enemy_spawn);

  /*
   * Debug info
   */
  igBegin("Debug", NULL, 0);

  igText("FPS: %.2f", cf_app_get_framerate());

  igText("Player Position: (%.2f, %.2f)", pos->x, pos->y);
  igText("Player Velocity: (%.2f, %.2f)", vel->x, vel->y);

  igText("Enemy.Max: %d", spawn->max_enemies);
  igText("Enemy.Count: %d", spawn->current_enemy_count);
  igText("Enemy.TimeSinceLastSpawn: %.2f", spawn->time_since_last_spawn);

  igText("Input.UP: %d", input->up);
  igText("Input.DOWN: %d", input->down);
  igText("Input.LEFT: %d", input->left);
  igText("Input.RIGHT: %d", input->right);
  igText("Input.SHOOT: %d", input->shoot);

  igDragFloat("Weapon.Cooldown", &weapon->cooldown, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
  igText("Weapon.TimeSinceShot: %f", weapon->time_since_shot);
  igEnd();

  /*
   * Window info
   */
  igBegin("Window", NULL, 0);
  int w = cf_app_get_width();
  int h = cf_app_get_height();
  igText("Screen: %dx%d", cf_display_width(g_state->display_id), cf_display_height(g_state->display_id));
  igText("Size: %dx%d", w, h);
  igText("Canvas: %dx%d", cf_app_get_canvas_width(), cf_app_get_canvas_height());
  igText("Canvas(logical): %dx%d", cf_div_v2_f(g_state->canvas_size, g_state->scale));
  igText("Game Scale: %.2f", g_state->scale);
  igText("DPI Scale: %.2f", cf_app_get_dpi_scale());
  igEnd();
}

EXPORT void game_render(void) {
  ecs_update_system(g_state->ecs, g_state->systems.render, CF_DELTA_TIME);

#ifdef DEBUG
  game_render_debug();
#endif
}

EXPORT void game_shutdown(void) {
  Platform *platform = g_state->platform;
  cf_destroy_arena(&g_state->scratch_arena);
  cf_destroy_arena(&g_state->stage_arena);
  cf_destroy_arena(&g_state->permanent_arena);
  ecs_free(g_state->ecs);
  platform->free_memory(g_state);
}

EXPORT void *game_state(void) { return g_state; }

EXPORT void game_hot_reload(void *game_state) {
  g_state = (GameState *)game_state;
  // Update system callbacks to new function addresses
  update_system_callbacks();
}
