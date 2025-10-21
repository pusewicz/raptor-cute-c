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
#include "floating_score.h"
#include "hit_particle.h"
#include "input.h"
#include "movement.h"
#include "player.h"
#include "render.h"
#include "scene.h"
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

    // Initialize the intro scene
    scene_init(SCENE_INTRO);
}

EXPORT bool game_update(void) {
    cf_arena_reset(&g_state->scratch_arena);

#ifdef DEBUG
    // Toggle debug mode
    if (cf_key_just_pressed(CF_KEY_G)) g_state->debug = !g_state->debug;
#endif

    // Delegate to current scene
    Scene current_scene = scene_get_current();
    if (current_scene.update) { return current_scene.update(); }

    return true;
}

EXPORT void game_render(void) {
    // Delegate to current scene
    Scene current_scene = scene_get_current();
    if (current_scene.render) { current_scene.render(); }
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
