#include "game.h"

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_audio.h>
#include <cute_c_runtime.h>
#include <cute_color.h>
#include <cute_defines.h>
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <dcimgui.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "../engine/log.h"
#include "asset/audio.h"
#include "asset/font.h"
#include "asset/sprite.h"
#include "background_scroll.h"
#include "collision.h"
#include "component.h"
#include "coroutine.h"
#include "enemy.h"
#include "explosion.h"
#include "explosion_particle.h"
#include "floating_score.h"
#include "hit_particle.h"
#include "input.h"
#include "movement.h"
#include "player.h"
#include "render.h"
#include "sprite.h"
#include "star_particle.h"

GameState* g_state = nullptr;

#ifdef _WIN32
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT
#endif

#define INIT_ENTITY_STORAGE(type, field, max)                                                \
    g_state->field            = cf_arena_alloc(&g_state->stage_arena, (max) * sizeof(type)); \
    g_state->field##_count    = 0;                                                           \
    g_state->field##_capacity = (max)

constexpr int PERMANENT_ARENA_SIZE    = CF_MB * 64;
constexpr int STAGE_ARENA_SIZE        = CF_MB * 64;
constexpr int SCRATCH_ARENA_SIZE      = CF_MB * 64;
constexpr int DEFAULT_ARENA_ALIGNMENT = 16;

constexpr int CANVAS_WIDTH            = 180;
constexpr int CANVAS_HEIGHT           = 320;

const int MAX_PLAYER_BULLETS          = 32;
const int MAX_ENEMIES                 = 32;
const int MAX_ENEMY_BULLETS           = 32;
const int MAX_HIT_PARTICLES           = 240;
const int MAX_EXPLOSION_PARTICLES     = 320;    // More particles for colorful explosions
const int MAX_EXPLOSIONS              = 32;
const int MAX_STAR_PARTICLES          = 4 * 4;  // 4 stars per 4 layers
const int MAX_FLOATING_SCORES         = 16;

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
    g_state->is_game_over         = false;
    g_state->lives                = 3;

    g_state->background_scroll    = make_background_scroll();
    g_state->player               = make_player(0.0f, -g_state->canvas_size.y / 3);

    init_coroutines();

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

    load_audio(&g_state->audio.music, "assets/music.ogg");
    load_audio(&g_state->audio.reveal, "assets/reveal.ogg");
    load_audio(&g_state->audio.game_over, "assets/game-over.ogg");
    load_audio(&g_state->audio.death, "assets/death.ogg");
    load_audio(&g_state->audio.laser_shoot, "assets/laser-shoot.ogg");
    load_audio(&g_state->audio.explosion, "assets/explosion.ogg");
    load_audio(&g_state->audio.hit_hurt, "assets/hit-hurt.ogg");
    load_sprite(&g_state->sprites.life_icon, "assets/life_icon.png");
    load_sprite(&g_state->sprites.game_over, "assets/gameover.png");

    cf_play_sound(g_state->audio.reveal, cf_sound_params_defaults());

    // Prepare the storage for player bullets
    INIT_ENTITY_STORAGE(PlayerBullet, player_bullets, MAX_PLAYER_BULLETS);
    INIT_ENTITY_STORAGE(Enemy, enemies, MAX_ENEMIES);
    INIT_ENTITY_STORAGE(EnemyBullet, enemy_bullets, MAX_ENEMY_BULLETS);
    INIT_ENTITY_STORAGE(HitParticle, hit_particles, MAX_HIT_PARTICLES);
    INIT_ENTITY_STORAGE(ExplosionParticle, explosion_particles, MAX_EXPLOSION_PARTICLES);
    INIT_ENTITY_STORAGE(Explosion, explosions, MAX_EXPLOSIONS);
    INIT_ENTITY_STORAGE(StarParticle, star_particles, MAX_STAR_PARTICLES);
    INIT_ENTITY_STORAGE(FloatingScore, floating_scores, MAX_FLOATING_SCORES);

    // Initialize shared particle sprite (1x1 white pixel)
    CF_Pixel particle_pixel = {
        .colors = {255, 255, 255, 255}
    };
    g_state->sprites.particle = cf_make_easy_sprite_from_pixels(&particle_pixel, 1, 1);

    // Initialize star particles
    init_star_particles();

    cf_music_play(g_state->audio.music, 0.5f);
}

EXPORT bool game_update(void) {
    cf_arena_reset(&g_state->scratch_arena);

    auto canvas_aabb = cf_make_aabb_center_half_extents(cf_v2(0, 0), cf_div_v2_f(g_state->canvas_size, 2.0f));

#ifdef DEBUG
    // Toggle debug mode
    if (cf_key_just_pressed(CF_KEY_G)) g_state->debug = !g_state->debug;
#endif

    update_input(&g_state->player.input);

    // Handle game over state
    if (g_state->is_game_over) {
        // Check for restart input (shoot button)
        if (g_state->player.input.shoot) {
            // TODO: Extract a generic init/reset function
            // Reset game state
            g_state->is_game_over              = false;
            g_state->lives                     = 3;
            g_state->score                     = 0;

            // Reset player
            g_state->player                    = make_player(0.0f, -g_state->canvas_size.y / 3);
            g_state->player_bullets_count      = 0;
            g_state->enemies_count             = 0;
            g_state->enemy_bullets_count       = 0;
            g_state->explosions_count          = 0;
            g_state->hit_particles_count       = 0;
            g_state->explosion_particles_count = 0;
            g_state->star_particles_count      = 0;
            g_state->floating_scores_count     = 0;

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
    update_hit_particles();
    update_explosion_particles();
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
    cleanup_explosion_particles();
    cleanup_player_bullets();
    cleanup_floating_scores();

    return true;
}

static void game_render_debug(void) {
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

EXPORT void game_render(void) {
#ifdef DEBUG
    if (g_state->debug) game_render_debug();
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

    // Render explosion particles with colors
    for (size_t i = 0; i < g_state->explosion_particles_count; i++) {
        cf_draw() {
            cf_draw_layer(Z_PARTICLES) {
                cf_draw_translate_v2(g_state->explosion_particles[i].position);
                cf_draw_scale(g_state->explosion_particles[i].size, g_state->explosion_particles[i].size);
                // Apply color to the sprite
                CF_Color color = g_state->explosion_particles[i].color;
                cf_draw_push_color(color);
                cf_draw_sprite(&g_state->explosion_particles[i].sprite);
                cf_draw_pop_color();
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

EXPORT void game_shutdown(void) {
    Platform* platform = g_state->platform;
    cf_destroy_arena(&g_state->scratch_arena);
    cf_destroy_arena(&g_state->stage_arena);
    cf_destroy_arena(&g_state->permanent_arena);
    platform->free_memory(g_state);
}

EXPORT void* game_state(void) { return g_state; }

EXPORT void game_hot_reload(void* game_state) {
    // Update global game state pointer
    g_state = (GameState*)game_state;

    // Re-initialize coroutines
    cleanup_coroutines();
    init_coroutines();
}
