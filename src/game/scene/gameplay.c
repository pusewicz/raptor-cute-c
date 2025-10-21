#include "gameplay.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_audio.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <dcimgui.h>
#include <stdio.h>

#include "../../engine/cute_macros.h"
#include "../../engine/game_state.h"
#include "../../engine/log.h"
#include "../background_scroll.h"
#include "../collision.h"
#include "../component.h"
#include "../coroutine.h"
#include "../enemy.h"
#include "../explosion.h"
#include "../floating_score.h"
#include "../hit_particle.h"
#include "../input.h"
#include "../movement.h"
#include "../player.h"
#include "../render.h"
#include "../star_particle.h"

void gameplay_init(void) {
    // Initialize gameplay scene
    // Most initialization is already done in game_init, but we can add scene-specific init here
}

bool gameplay_update(void) {
    auto canvas_aabb = cf_make_aabb_center_half_extents(cf_v2(0, 0), cf_div_v2_f(g_state->canvas_size, 2.0f));

    update_input(&g_state->player.input);

    // Handle game over state
    if (g_state->is_game_over) {
        // Check for restart input (shoot button)
        if (g_state->player.input.shoot) {
            // TODO: Extract a generic init/reset function
            // Reset game state
            g_state->is_game_over          = false;
            g_state->lives                 = 3;
            g_state->score                 = 0;

            // Reset player
            g_state->player                = make_player(0.0f, -g_state->canvas_size.y / 3);
            g_state->player_bullets_count  = 0;
            g_state->enemies_count         = 0;
            g_state->enemy_bullets_count   = 0;
            g_state->explosions_count      = 0;
            g_state->hit_particles_count   = 0;
            g_state->star_particles_count  = 0;
            g_state->floating_scores_count = 0;

            // Re-initialize star particles
            init_star_particles();

            // Restart coroutines
            cleanup_coroutines();
            init_coroutines();
        }
        return true;
    }
    update_player(&g_state->player);
    // Update player movement
    update_movement(&g_state->player.position, &g_state->player.velocity);

    // Update player bullets
    for (size_t i = 0; i < g_state->player_bullets_count; i++) {
        update_movement(&g_state->player_bullets[i].position, &g_state->player_bullets[i].velocity);

        // Mark bullet as destroyed when out of screen bounds
        if (g_state->player_bullets[i].position.y > g_state->canvas_size.y * 0.5f) {
            g_state->player_bullets[i].is_alive = false;
        }
    }
    // Update enemies
    for (size_t i = 0; i < g_state->enemies_count; i++) {
        update_movement(&g_state->enemies[i].position, &g_state->enemies[i].velocity);
        update_enemy(&g_state->enemies[i]);  // TODO: Rename to update_enemy_weapon

        // Mark enemy as destroyed when out of screen bounds
        auto enemy_aabb =
            cf_make_aabb_center_half_extents(g_state->enemies[i].position, g_state->enemies[i].collider.half_extents);
        if (!cf_aabb_to_aabb(canvas_aabb, enemy_aabb)) { g_state->enemies[i].is_alive = false; }
    }
    // Update enemy bullets
    for (size_t i = 0; i < g_state->enemy_bullets_count; i++) {
        update_movement(&g_state->enemy_bullets[i].position, &g_state->enemy_bullets[i].velocity);

        // Mark bullet as destroyed when out of screen bounds
        auto bullet_aabb = cf_make_aabb_center_half_extents(
            g_state->enemy_bullets[i].position, g_state->enemy_bullets[i].collider.half_extents
        );
        if (!cf_aabb_to_aabb(canvas_aabb, bullet_aabb)) { g_state->enemy_bullets[i].is_alive = false; }
    }
    // Update hit particles
    for (size_t i = 0; i < g_state->hit_particles_count; i++) {
        update_movement(&g_state->hit_particles[i].position, &g_state->hit_particles[i].velocity);

        // Mark particle as destroyed when out of screen bounds
        if (g_state->hit_particles[i].position.x < canvas_aabb.min.x ||
            g_state->hit_particles[i].position.x > canvas_aabb.max.x ||
            g_state->hit_particles[i].position.y < canvas_aabb.min.y ||
            g_state->hit_particles[i].position.y > canvas_aabb.max.y) {
            g_state->hit_particles[i].is_alive = false;
        }
    }
    update_particles();
    update_star_particles();
    update_floating_scores();

    // TODO: Decide where to move this
    // Clamp player position to canvas bounds
    g_state->player.position.x =
        cf_clamp(g_state->player.position.x, -g_state->canvas_size.x / 2.0f, g_state->canvas_size.x / 2.0f);
    g_state->player.position.y =
        cf_clamp(g_state->player.position.y, -g_state->canvas_size.y / 2.0f, g_state->canvas_size.y / 2.0f);

    update_background_scroll();
    update_collision();
    update_coroutine();

    cleanup_enemies();
    cleanup_enemy_bullets();
    cleanup_explosions();
    cleanup_hit_particles();
    cleanup_player_bullets();
    cleanup_floating_scores();

    return true;
}

static void gameplay_render_debug(void) {
    auto weapon = &g_state->player.weapon;
    auto pos    = &g_state->player.position;
    auto vel    = &g_state->player.velocity;
    auto input  = &g_state->player.input;

    ImGui_Begin("Debug Menu", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    {
        if (ImGui_CollapsingHeader("Debug", true)) {
            ImGui_Checkbox("Draw Bounding Boxes", &g_state->debug_bounding_boxes);
        }

        if (ImGui_CollapsingHeader("Player", true)) {
            ImGui_Text("Position: (%.2f, %.2f)", pos->x, pos->y);
            ImGui_Text("Velocity: (%.2f, %.2f)", vel->x, vel->y);
            ImGui_Text("Up: %s", input->up ? "Y" : "N");
            ImGui_Text("Down: %s", input->down ? "Y" : "N");
            ImGui_Text("Left: %s", input->left ? "Y" : "N");
            ImGui_Text("Right: %s", input->right ? "Y" : "N");
            ImGui_Text("Shoot: %s", input->shoot ? "Y" : "N");
        }

        if (ImGui_CollapsingHeader("Entity Counts", true)) {
            ImGui_Text("Enemies: %zu", g_state->enemies_count);
            ImGui_Text("EnemyBullets: %zu", g_state->enemy_bullets_count);
            ImGui_Text("Explosions: %zu", g_state->explosions_count);
            ImGui_Text("HitParticles: %zu", g_state->hit_particles_count);
            ImGui_Text("PlayerBullets: %zu", g_state->player_bullets_count);
        }

        if (ImGui_CollapsingHeader("Weapon", true)) {
            ImGui_DragFloat("Cooldown", &weapon->cooldown);
            ImGui_Text("Time Since Last Shot: %.2f", weapon->time_since_shot);
        }

        if (ImGui_CollapsingHeader("Performance", true)) { ImGui_Text("FPS: %.2f", cf_app_get_framerate()); }

        if (ImGui_CollapsingHeader("Window", true)) {
            ImGui_Text("Screen: %dx%d", cf_display_width(g_state->display_id), cf_display_height(g_state->display_id));
            ImGui_Text("Size: %dx%d", cf_app_get_width(), cf_app_get_height());
            ImGui_Text("Canvas: %dx%d", cf_app_get_canvas_width(), cf_app_get_canvas_height());
            ImGui_Text("Canvas(logical): %.0fx%.0f", g_state->canvas_size.x, g_state->canvas_size.y);
            ImGui_Text("Game Scale: %.2f", g_state->scale);
            ImGui_Text("DPI Scale: %.2f", cf_app_get_dpi_scale());
        }
    }
    ImGui_End();

    if (g_state->debug_bounding_boxes) {
        // Draw on top of everything
        cf_draw_layer(Z_MAX) {
            RENDER_DEBUG_BBOXES(g_state->enemies, g_state->enemies_count, position, collider);
            RENDER_DEBUG_BBOXES(g_state->player_bullets, g_state->player_bullets_count, position, collider);
            RENDER_DEBUG_BBOXES(g_state->enemy_bullets, g_state->enemy_bullets_count, position, collider);
            {
                auto entity        = &g_state->player;
                auto aabb_collider = cf_make_aabb_center_half_extents(entity->position, entity->collider.half_extents);

                cf_draw() {
                    cf_draw_color(cf_color_blue()) { cf_draw_quad(aabb_collider, 0, 0); }
                }
            }
        }
    }
}

void gameplay_render(void) {
#ifdef DEBUG
    if (g_state->debug) gameplay_render_debug();
#endif

    render_background_scroll();
    render_star_particles();

    // Show game over screen
    if (g_state->is_game_over) {
        cf_draw() {
            cf_draw_layer(Z_UI) {
                // Draw game over sprite centered
                cf_draw_sprite(&g_state->sprites.game_over);
            }
        }

        return;
    }

    render_player(&g_state->player);
    RENDER_ENTITY_ARRAY(g_state->enemies, g_state->enemies_count, sprite, position, z_index);
    RENDER_ENTITY_ARRAY(g_state->enemy_bullets, g_state->enemy_bullets_count, sprite, position, z_index);
    RENDER_ENTITY_ARRAY(g_state->explosions, g_state->explosions_count, sprite, position, z_index);
    RENDER_ENTITY_ARRAY(g_state->player_bullets, g_state->player_bullets_count, sprite, position, z_index);

    // Custom code for particles draw
    for (size_t i = 0; i < g_state->hit_particles_count; i++) {
        cf_draw() {
            cf_draw_layer(Z_PARTICLES) {
                cf_draw_translate_v2(g_state->hit_particles[i].position);
                cf_draw_scale(g_state->hit_particles[i].size, g_state->hit_particles[i].size);
                cf_draw_sprite(&g_state->hit_particles[i].sprite);
            }
        }
    }

    render_floating_scores();

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

        // Render life icons
        const int icon_margin_right  = 4;
        const int icon_margin_bottom = 4;
        float     icon_width         = (float)g_state->sprites.life_icon.w;
        float     icon_height        = (float)g_state->sprites.life_icon.h;
        float     canvas_half_width  = (float)cf_app_get_canvas_width() / 2 / g_state->scale;
        float     canvas_half_height = (float)cf_app_get_canvas_height() / 2 / g_state->scale;

        for (int i = 0; i < g_state->lives; i++) {
            float x = canvas_half_width - icon_margin_right - (i + 1) * (icon_width) + icon_width / 2;
            float y = -canvas_half_height + icon_margin_bottom + icon_height / 4;
            cf_draw() {
                cf_draw_translate_v2(cf_v2(x, y));
                cf_draw_sprite(&g_state->sprites.life_icon);
            }
        }
    }
}

void gameplay_cleanup(void) {
    // Cleanup gameplay scene resources here
}
