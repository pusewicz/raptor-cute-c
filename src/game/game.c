#include "game.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_c_runtime.h>
#include <cute_defines.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_time.h>
#include <dcimgui.h>
#include <pico_ecs.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "../engine/game_state.h"
#include "../engine/log.h"
#include "ecs.h"
#include "factories.h"

GameState* g_state = nullptr;

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

constexpr int PERMANENT_ARENA_SIZE    = CF_MB * 64;
constexpr int STAGE_ARENA_SIZE        = CF_MB * 64;
constexpr int SCRATCH_ARENA_SIZE      = CF_MB * 64;
constexpr int DEFAULT_ARENA_ALIGNMENT = 16;

enum { CANVAS_WIDTH = 180, CANVAS_HEIGHT = 320 };

EXPORT void game_init(Platform* platform) {
    g_state = platform->allocate_memory(sizeof(GameState));

    const int scale               = 3;
    g_state->display_id           = cf_default_display();
    g_state->platform             = platform;
    g_state->canvas_size          = cf_v2(CANVAS_WIDTH, CANVAS_HEIGHT);
    g_state->scale                = scale;
    g_state->permanent_arena      = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, PERMANENT_ARENA_SIZE);
    g_state->stage_arena          = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, STAGE_ARENA_SIZE);
    g_state->scratch_arena        = cf_make_arena(DEFAULT_ARENA_ALIGNMENT, SCRATCH_ARENA_SIZE);
    g_state->rnd                  = cf_rnd_seed((uint32_t)time(nullptr));
    g_state->debug_bounding_boxes = false;

    // Initialize ECS
    g_state->ecs = ecs_new(96, nullptr);
    init_ecs();
    g_state->entities.background_scroll = make_background_scroll();
    g_state->entities.player            = make_player(0.0f, 0.0f);
    g_state->entities.enemy_spawner     = make_enemy_spawner();

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

    ECS_UPDATE_SYSTEM(background_scroll);
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
    InputComponent*      input  = ECS_GET(g_state->entities.player, InputComponent);
    WeaponComponent*     weapon = ECS_GET(g_state->entities.player, WeaponComponent);
    PositionComponent*   pos    = ECS_GET(g_state->entities.player, PositionComponent);
    VelocityComponent*   vel    = ECS_GET(g_state->entities.player, VelocityComponent);
    EnemySpawnComponent* spawn  = ECS_GET(g_state->entities.enemy_spawner, EnemySpawnComponent);

    /*
     * Debug info
     */
    ImGui_Begin("Debug", nullptr, 0);

    ImGui_Text("FPS: %.2f", cf_app_get_framerate());
    ImGui_Text("Draw Calls: %d", g_state->draw_calls);

    ImGui_Text("Player Position: (%.2f, %.2f)", pos->x, pos->y);
    ImGui_Text("Player Velocity: (%.2f, %.2f)", vel->x, vel->y);

    ImGui_Text("Enemy.Max: %d", spawn->max_enemies);
    ImGui_Text("Enemy.Count: %d", spawn->current_enemy_count);
    ImGui_Text("Enemy.TimeSinceLastSpawn: %.2f", spawn->time_since_last_spawn);

    ImGui_Text("Input.UP: %d", input->up);
    ImGui_Text("Input.DOWN: %d", input->down);
    ImGui_Text("Input.LEFT: %d", input->left);
    ImGui_Text("Input.RIGHT: %d", input->right);
    ImGui_Text("Input.SHOOT: %d", input->shoot);

    ImGui_Checkbox("Draw Bounding Boxes", &g_state->debug_bounding_boxes);
    ImGui_End();

    ImGui_Begin("Weapon", nullptr, 0);
    ImGui_DragFloat("Cooldown", &weapon->cooldown);
    ImGui_Text("Time Since Last Shot: %.2f", weapon->time_since_shot);
    ImGui_End();

    /*
     * Window info
     */
    ImGui_Begin("Window", nullptr, 0);
    ImGui_Text("Screen: %dx%d", cf_display_width(g_state->display_id), cf_display_height(g_state->display_id));
    ImGui_Text("Size: %dx%d", cf_app_get_width(), cf_app_get_height());
    ImGui_Text("Canvas: %dx%d", cf_app_get_canvas_width(), cf_app_get_canvas_height());
    ImGui_Text("Canvas(logical): %.0fx%.0f", g_state->canvas_size.x, g_state->canvas_size.y);
    ImGui_Text("Game Scale: %.2f", g_state->scale);
    ImGui_Text("DPI Scale: %.2f", cf_app_get_dpi_scale());
    ImGui_End();
}

EXPORT void game_render(void) {
    ECS_UPDATE_SYSTEM(render);

#ifdef DEBUG
    game_render_debug();
#endif
}

EXPORT void game_shutdown(void) {
    Platform* platform = g_state->platform;
    cf_destroy_arena(&g_state->scratch_arena);
    cf_destroy_arena(&g_state->stage_arena);
    cf_destroy_arena(&g_state->permanent_arena);
    ecs_free(g_state->ecs);
    platform->free_memory(g_state);
}

EXPORT void* game_state(void) { return g_state; }

EXPORT void game_hot_reload(void* game_state) {
    // Update global game state pointer
    g_state = (GameState*)game_state;

    // Update system callbacks to new function addresses
    update_ecs_system_callbacks();
}

EXPORT void game_set_draw_calls(int draw_calls) {
    if (g_state) {
        g_state->draw_calls = draw_calls;
    }
}
