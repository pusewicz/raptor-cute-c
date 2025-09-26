#include "game.h"

#include "../engine/common.h"
#include "../engine/game_state.h"
#include "../engine/log.h"
#include "ecs.h"
#include "factories.h"

#include <cimgui.h>
#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_c_runtime.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_time.h>
#include <pico_ecs.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

GameState *g_state = nullptr;

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

#define PERMANENT_ARENA_SIZE    MiB(64)
#define STAGE_ARENA_SIZE        MiB(64)
#define SCRATCH_ARENA_SIZE      MiB(64)
#define DEFAULT_ARENA_ALIGNMENT 16

enum { CANVAS_WIDTH = 180, CANVAS_HEIGHT = 320 };

EXPORT void game_init(Platform *platform) {
  g_state = platform->allocate_memory(sizeof(GameState));

  const int scale = 3;

  g_state->display_id           = cf_default_display();
  g_state->platform             = platform;
  g_state->canvas_size          = cf_v2(CANVAS_WIDTH, CANVAS_HEIGHT);
  g_state->scale                = scale;
  g_state->permanent_arena      = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, PERMANENT_ARENA_SIZE);
  g_state->stage_arena          = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, STAGE_ARENA_SIZE);
  g_state->scratch_arena        = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, SCRATCH_ARENA_SIZE);
  g_state->ecs                  = ecs_new(1024, nullptr);
  g_state->rnd                  = cf_rnd_seed((uint32_t)time(nullptr));
  g_state->debug_bounding_boxes = false;

  // Initialize component and system IDs to 0
  g_state->components = (Components){0};
  g_state->systems    = (Systems){0};

  init_ecs();

  g_state->player_entity        = make_player(0.0f, 0.0f);
  g_state->enemy_spawner_entity = make_enemy_spawner();

  if (!validate_game_state()) {
    APP_FATAL("GameState validation failed in game_init");
    CF_ASSERT(false);
  }

  cf_app_set_canvas_size((int)g_state->canvas_size.x * g_state->scale, (int)g_state->canvas_size.y * g_state->scale);
  cf_app_set_size((int)g_state->canvas_size.x * g_state->scale, (int)g_state->canvas_size.y * g_state->scale);
  cf_app_center_window();
#ifdef DEBUG
  cf_app_init_imgui();
#endif
}

EXPORT bool game_update(void) {
  cf_arena_reset(&g_state->scratch_arena);

  ECS_UPDATE_SYSTEM(input);
  ECS_UPDATE_SYSTEM(movement);
  ECS_UPDATE_SYSTEM(weapon);
  ECS_UPDATE_SYSTEM(enemy_spawn);
  ECS_UPDATE_SYSTEM(collision);
  ECS_UPDATE_SYSTEM(boundary);
#ifdef DEBUG
  ECS_UPDATE_SYSTEM(debug_bounding_boxes);
#endif

  return true;
}

static void game_render_debug(void) {
  InputComponent      *input  = ECS_GET(g_state->player_entity, InputComponent);
  WeaponComponent     *weapon = ECS_GET(g_state->player_entity, WeaponComponent);
  PositionComponent   *pos    = ECS_GET(g_state->player_entity, PositionComponent);
  VelocityComponent   *vel    = ECS_GET(g_state->player_entity, VelocityComponent);
  EnemySpawnComponent *spawn  = ECS_GET(g_state->enemy_spawner_entity, EnemySpawnComponent);

  /*
   * Debug info
   */
  igBegin("Debug", nullptr, 0);

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

  igCheckbox("Draw Bounding Boxes", &g_state->debug_bounding_boxes);
  igEnd();

  igBegin("Weapon", nullptr, 0);
  igDragFloat("Cooldown", &weapon->cooldown, 0.01f, 0.0f, 1.0f, "%.2f", 1.0f);
  igText("Time Since Last Shot: %.2f", weapon->time_since_shot);
  igEnd();

  /*
   * Window info
   */
  igBegin("Window", nullptr, 0);
  igText("Screen: %dx%d", cf_display_width(g_state->display_id), cf_display_height(g_state->display_id));
  igText("Size: %dx%d", cf_app_get_width(), cf_app_get_height());
  igText("Canvas: %dx%d", cf_app_get_canvas_width(), cf_app_get_canvas_height());
  igText("Canvas(logical): %.0fx%.0f", g_state->canvas_size.x, g_state->canvas_size.y);
  igText("Game Scale: %.2f", g_state->scale);
  igText("DPI Scale: %.2f", cf_app_get_dpi_scale());
  igEnd();
}

EXPORT void game_render(void) {
  ECS_UPDATE_SYSTEM(render);

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
  // Update global game state pointer
  g_state = (GameState *)game_state;

  // Update system callbacks to new function addresses
  update_ecs_system_callbacks();
}
