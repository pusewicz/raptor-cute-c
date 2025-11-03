#include "enemy.h"

#include <cute_c_runtime.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_time.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "asset/audio.h"
#include "asset/sprite.h"
#include "component.h"

constexpr float ENEMY_DEFAULT_SPEED = 0.5f;

Enemy make_enemy_of_type(CF_V2 position, EnemyType type) {
    // Sprite
    Sprite sprite;
    int    score_value;
    int    health_value;

    switch (type) {
        case ENEMY_TYPE_ALAN:
            sprite       = SPRITE_ALAN;
            score_value  = 100;
            health_value = 1;
            break;
        case ENEMY_TYPE_BON_BON:
            sprite       = SPRITE_BON_BON;
            score_value  = 150;
            health_value = 2;
            break;
        case ENEMY_TYPE_LIPS:
            sprite       = SPRITE_LIPS;
            score_value  = 200;
            health_value = 3;
            break;
        default:
            sprite       = SPRITE_ALAN;
            score_value  = 100;
            health_value = 1;
            break;
    }

    Enemy enemy = (Enemy){
        .position = position,
        .velocity = cf_v2(0, -ENEMY_DEFAULT_SPEED),
        .z_index  = Z_SPRITES,
        .score    = score_value,
        .is_alive = true,
        .type     = type,
    };

    // Sprite
    enemy.sprite                = get_sprite(sprite);

    // Collider
    enemy.collider.half_extents = cf_v2(enemy.sprite.w / 3.0f, enemy.sprite.h / 3.0f);

    // Health
    enemy.health.current = enemy.health.maximum = health_value;

    // Weapon
    enemy.cooldown                              = cf_rnd_range_float(&g_state->rnd, 2.5f, 6.5f);
    enemy.time_since_shot                       = cf_rnd_range_float(&g_state->rnd, 0.0f, enemy.cooldown);
    enemy.shoot_chance                          = 0.3f;  // 30% chance to shoot when cooldown ready

    return enemy;
}

Enemy make_random_enemy(CF_V2 position) {
    EnemyType types[] = {ENEMY_TYPE_ALAN, ENEMY_TYPE_BON_BON, ENEMY_TYPE_LIPS};
    int       type    = cf_rnd_range_int(&g_state->rnd, 0, 2);
    return make_enemy_of_type(position, types[type]);
}

constexpr float ENEMY_BULLET_DEFAULT_SPEED = 1.22f;

EnemyBullet make_enemy_bullet(CF_V2 position, CF_V2 direction) {
    EnemyBullet bullet = (EnemyBullet){
        .is_alive = true,
        .position = position,
    };

    // Velocity
    bullet.velocity.x            = ENEMY_BULLET_DEFAULT_SPEED * direction.x;
    bullet.velocity.y            = ENEMY_BULLET_DEFAULT_SPEED * direction.y;

    // Sprite
    bullet.sprite                = get_sprite(SPRITE_ENEMY_BULLET);
    bullet.z_index               = Z_SPRITES;

    // Collider
    bullet.collider.half_extents = cf_v2(bullet.sprite.w / 4.2f, bullet.sprite.h / 4.2f);

    return bullet;
}

void spawn_enemy_bullet(EnemyBullet bullet) {
    CF_ASSERT(g_state->enemy_bullets);
    CF_ASSERT(g_state->enemy_bullets_count < g_state->enemy_bullets_capacity);

    g_state->enemy_bullets[g_state->enemy_bullets_count++] = bullet;
}

void spawn_enemy(Enemy enemy) {
    CF_ASSERT(g_state->enemies);
    CF_ASSERT(g_state->enemies_count < g_state->enemies_capacity - 1);

    g_state->enemies[g_state->enemies_count++] = enemy;
}

void update_enemy(Enemy* enemy) {
    // Update time since shot
    enemy->time_since_shot += CF_DELTA_TIME;

    // Check if cooldown is ready
    if (enemy->time_since_shot >= enemy->cooldown) {
        // Random chance to shoot
        float random_value = cf_rnd_float(&g_state->rnd);
        if (random_value < enemy->shoot_chance) {
            enemy->time_since_shot = 0.0f;

            // Shoot downward (toward player)
            spawn_enemy_bullet(make_enemy_bullet(enemy->position, cf_v2(0, -1)));

            play_sound(SOUND_LASER);
        }
    }
}

void cleanup_enemies(void) {
    int write_idx = 0;
    for (size_t i = 0; i < g_state->enemies_count; i++) {
        if (g_state->enemies[i].is_alive) { g_state->enemies[write_idx++] = g_state->enemies[i]; }
    }
    g_state->enemies_count = write_idx;
}

void cleanup_enemy_bullets(void) {
    int write_idx = 0;
    for (size_t i = 0; i < g_state->enemy_bullets_count; i++) {
        if (g_state->enemy_bullets[i].is_alive) { g_state->enemy_bullets[write_idx++] = g_state->enemy_bullets[i]; }
    }
    g_state->enemy_bullets_count = write_idx;
}
