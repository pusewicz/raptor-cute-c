#pragma once

#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_audio.h>
#include <cute_color.h>
#include <cute_coroutine.h>
#include <cute_draw.h>
#include <cute_graphics.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "../game/background_scroll.h"
#include "../game/enemy.h"
#include "../game/explosion.h"
#include "../game/explosion_particle.h"
#include "../game/floating_score.h"
#include "../game/hit_particle.h"
#include "../game/player.h"
#include "../game/player_bullet.h"
#include "../game/star_particle.h"

typedef struct Platform Platform;

/*
 * Game State
 *
 * Should be validated with validate_game_state() before use
 */
typedef struct GameState {
    Platform*    platform;
    CF_V2        canvas_size;
    float        scale;  // For resolution independence
    CF_Arena     permanent_arena;
    CF_Arena     stage_arena;
    CF_Arena     scratch_arena;
    CF_DisplayID display_id;
    CF_Rnd       rnd;
    int          score;
    int          lives;

    CF_Shader recolor;

    BackgroundScroll background_scroll;

    Player        player;
    PlayerBullet* player_bullets;
    size_t        player_bullets_count;
    size_t        player_bullets_capacity;

    Enemy* enemies;
    size_t enemies_count;
    size_t enemies_capacity;

    EnemyBullet* enemy_bullets;
    size_t       enemy_bullets_count;
    size_t       enemy_bullets_capacity;

    Explosion* explosions;
    size_t     explosions_count;
    size_t     explosions_capacity;

    HitParticle* hit_particles;
    size_t       hit_particles_count;
    size_t       hit_particles_capacity;

    ExplosionParticle* explosion_particles;
    size_t             explosion_particles_count;
    size_t             explosion_particles_capacity;

    StarParticle* star_particles;
    size_t        star_particles_count;
    size_t        star_particles_capacity;

    FloatingScore* floating_scores;
    size_t         floating_scores_count;
    size_t         floating_scores_capacity;

    struct {
        CF_Coroutine spawner;
    } coroutines;

    struct {
        CF_Audio music;
        CF_Audio laser_shoot;
        CF_Audio explosion;
        CF_Audio hit_hurt;
        CF_Audio reveal;
        CF_Audio game_over;
        CF_Audio death;
    } audio;

    struct {
        CF_Sprite life_icon;
        CF_Sprite particle;
        CF_Sprite game_over;
    } sprites;

    bool is_game_over;
    bool debug;  // Enable ImGUI debug pane
    bool debug_bounding_boxes;
} GameState;

extern GameState* g_state;

bool validate_game_state(void);
