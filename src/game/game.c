#include "game.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_c_runtime.h>
#include <cute_color.h>
#include <cute_defines.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <dcimgui.h>
#include <pico_ecs.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "../engine/log.h"
#include "asset/font.h"
#include "asset/sprite.h"
#include "coroutine.h"
#include "ecs.h"
#include "event.h"
#include "factory.h"

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

constexpr int CANVAS_WIDTH            = 180;
constexpr int CANVAS_HEIGHT           = 320;

EXPORT void game_init(Platform* platform) {
    g_state                       = platform->allocate_memory(sizeof(GameState));

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
    g_state->lives                = 3;

    // Initialize ECS
    g_state->ecs                  = ecs_new(96, nullptr);
    init_ecs();
    g_state->entities.background_scroll = make_background_scroll();
    g_state->entities.player            = make_player(0.0f, -g_state->canvas_size.y / 3);

    init_coroutines();
    init_events();

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

    load_font("assets/tiny-and-chunky.ttf", "TinyAndChunky");
}

EXPORT bool game_update(void) {
    cf_arena_reset(&g_state->scratch_arena);

    auto ret = ecs_update_systems(g_state->ecs, CF_DELTA_TIME);

    return ret == 0;
}

static void game_render_debug(void) {
    auto weapon = ECS_GET(g_state->entities.player, WeaponComponent);
    auto pos    = ECS_GET(g_state->entities.player, PositionComponent);
    auto vel    = ECS_GET(g_state->entities.player, VelocityComponent);

    ImGui_Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    if (ImGui_CollapsingHeader("Debug", true)) {
        ImGui_Checkbox("Draw Bounding Boxes", &g_state->debug_bounding_boxes);
    }

    if (ImGui_CollapsingHeader("Player", true)) {
        ImGui_Text("Position: (%.2f, %.2f)", pos->x, pos->y);
        ImGui_Text("Velocity: (%.2f, %.2f)", vel->x, vel->y);
    }

    if (ImGui_CollapsingHeader("Weapon", true)) {
        ImGui_DragFloat("Cooldown", &weapon->cooldown);
        ImGui_Text("Time Since Last Shot: %.2f", weapon->time_since_shot);
    }

    if (ImGui_CollapsingHeader("Performance", true)) { ImGui_Text("FPS: %.2f", 1.0f / CF_DELTA_TIME); }

    if (ImGui_CollapsingHeader("Window", true)) {
        ImGui_Text("Screen: %dx%d", cf_display_width(g_state->display_id), cf_display_height(g_state->display_id));
        ImGui_Text("Size: %dx%d", cf_app_get_width(), cf_app_get_height());
        ImGui_Text("Canvas: %dx%d", cf_app_get_canvas_width(), cf_app_get_canvas_height());
        ImGui_Text("Canvas(logical): %.0fx%.0f", g_state->canvas_size.x, g_state->canvas_size.y);
        ImGui_Text("Game Scale: %.2f", g_state->scale);
        ImGui_Text("DPI Scale: %.2f", cf_app_get_dpi_scale());
    }
    ImGui_End();
}

EXPORT void game_render(void) {
    // Render UI
    char score_text[6 + 1];

    cf_draw() {
        cf_font("TinyAndChunky") {
            cf_push_font_size(7);
            snprintf(score_text, 7, "%06d", g_state->score);
            float     text_width   = cf_text_width(score_text, -1);
            float     text_height  = cf_text_height(score_text, -1);
            float     offset_x     = (float)cf_app_get_canvas_width() / 2 / g_state->scale - text_width;
            float     offset_y     = (float)cf_app_get_canvas_height() / 2 / g_state->scale + text_height / 2;
            const int margin_top   = 4;
            const int margin_right = 4;

            cf_draw_color(cf_make_color_rgb(20, 91, 132)) {
                cf_draw_text(score_text, cf_v2(offset_x + 1 - margin_right, offset_y - 1 - margin_top), -1);
            }
            cf_draw_color(cf_color_white()) {
                cf_draw_text(score_text, cf_v2(offset_x - margin_right, offset_y - margin_top), -1);
            }
        }

        CF_Sprite life_icon = {0};
        load_sprite(&life_icon, "assets/life_icon.png");

        // Render life icons
        const int icon_margin_right  = 4;
        const int icon_margin_bottom = 4;
        float     icon_width         = (float)life_icon.w;
        float     icon_height        = (float)life_icon.h;
        float     canvas_half_width  = (float)cf_app_get_canvas_width() / 2 / g_state->scale;
        float     canvas_half_height = (float)cf_app_get_canvas_height() / 2 / g_state->scale;

        for (int i = 0; i < g_state->lives; i++) {
            float x = canvas_half_width - icon_margin_right - (i + 1) * (icon_width) + icon_width / 2;
            float y = -canvas_half_height + icon_margin_bottom + icon_height / 4;
            cf_draw() {
                cf_draw_translate_v2(cf_v2(x, y));
                cf_draw_sprite(&life_icon);
            }
        }
    }
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

    // Re-initialize coroutines
    cleanup_coroutines();
    init_coroutines();
    init_events();
}
