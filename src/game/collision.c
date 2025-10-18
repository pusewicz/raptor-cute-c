#include "collision.h"

#include <cute_audio.h>
#include <cute_math.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "enemy.h"
#include "explosion.h"
#include "hit_particle.h"
#include "player.h"

static void player_bullets_vs_enemies(
    size_t       player_bullets_count,
    PlayerBullet player_bullets[static player_bullets_count],
    size_t       enemies_count,
    Enemy        enemies[static enemies_count]
) {
    for (size_t i = 0; i < player_bullets_count; ++i) {
        if (!player_bullets[i].is_alive) { continue; }

        for (size_t j = 0; j < enemies_count; ++j) {
            if (!enemies[j].is_alive) { continue; }

            auto bullet      = &player_bullets[i];
            auto enemy       = &enemies[j];

            auto bullet_aabb = cf_make_aabb_center_half_extents(bullet->position, bullet->collider.half_extents);
            auto enemy_aabb  = cf_make_aabb_center_half_extents(enemy->position, enemy->collider.half_extents);

            if (cf_aabb_to_aabb(bullet_aabb, enemy_aabb)) {
                // Damage the enemy
                enemy->health.current -= 1;

                // Destroy bullet
                bullet->is_alive = false;

                // If enemy survives, push it upwards and spawn particles
                if (enemy->health.current > 0) {
                    enemy->position.y += 5.0f;  // Push upwards by 5 pixels
                } else {
                    g_state->score += enemy->score;
                    // Destroy enemy
                    enemy->is_alive = false;

                    spawn_explosion(make_explosion(enemy->position.x, enemy->position.y));
                    cf_play_sound(g_state->audio.explosion, cf_sound_params_defaults());
                }

                // Get bullet direction from velocity and reverse it
                auto bullet_dir = cf_mul(cf_norm(bullet->velocity), -1.0f);

                // Spawn white debris particles opposite to the bullet's direction
                spawn_hit_particle_burst(5, enemy->position, bullet_dir);
            }
        }
    }
}

static void player_vs_enemies(const Player* player, size_t enemies_count, Enemy enemies[static enemies_count]) {
    if (player->is_alive && !player->is_invincible) {
        auto player_aabb = cf_make_aabb_center_half_extents(player->position, player->collider.half_extents);

        for (size_t i = 0; i < enemies_count; ++i) {
            if (!enemies[i].is_alive) { continue; }

            auto enemy      = &enemies[i];
            auto enemy_aabb = cf_make_aabb_center_half_extents(enemy->position, enemy->collider.half_extents);

            if (cf_aabb_to_aabb(player_aabb, enemy_aabb)) {
                enemy->is_alive = false;
                damage_player();
            }
        }
    }
}

static void player_vs_enemy_bullets(
    const Player* player, size_t enemy_bullets_count, EnemyBullet enemy_bullets[static enemy_bullets_count]
) {
    if (player->is_alive && !player->is_invincible) {
        auto player_aabb = cf_make_aabb_center_half_extents(player->position, player->collider.half_extents);

        for (size_t i = 0; i < enemy_bullets_count; ++i) {
            if (!enemy_bullets[i].is_alive) { continue; }

            auto enemy_bullet = &enemy_bullets[i];
            auto enemy_aabb =
                cf_make_aabb_center_half_extents(enemy_bullet->position, enemy_bullet->collider.half_extents);

            if (cf_aabb_to_aabb(player_aabb, enemy_aabb)) {
                enemy_bullet->is_alive = false;
                damage_player();
            }
        }
    }
}

void update_collision(void) {
    player_bullets_vs_enemies(
        g_state->player_bullets_count, g_state->player_bullets, g_state->enemies_count, g_state->enemies
    );
    // Player vs Enemies
    player_vs_enemies(&g_state->player, g_state->enemies_count, g_state->enemies);
    // Enemy bullet vs Player collision
    player_vs_enemy_bullets(&g_state->player, g_state->enemy_bullets_count, g_state->enemy_bullets);
}
