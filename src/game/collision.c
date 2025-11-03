#include "collision.h"

#include <cute_math.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "asset/audio.h"
#include "enemy.h"
#include "explosion.h"
#include "explosion_particle.h"
#include "floating_score.h"
#include "hit_particle.h"
#include "player.h"
#include "player_bullet.h"
#include "screenshake.h"

static void player_bullets_vs_enemies(
    const size_t player_bullets_count,
    PlayerBullet player_bullets[static restrict player_bullets_count],
    const size_t enemies_count,
    Enemy        enemies[static restrict enemies_count]
) {
    if (player_bullets_count == 0 || enemies_count == 0) { return; }

    for (size_t i = 0; i < player_bullets_count; ++i) {
        auto bullet = &player_bullets[i];
        if (!bullet->is_alive) { continue; }

        // Calculate bullet AABB once per bullet (not per enemy)
        auto bullet_aabb = cf_make_aabb_center_half_extents(bullet->position, bullet->collider.half_extents);

        for (size_t j = 0; j < enemies_count; ++j) {
            if (!enemies[j].is_alive) { continue; }

            auto enemy      = &enemies[j];
            auto enemy_aabb = cf_make_aabb_center_half_extents(enemy->position, enemy->collider.half_extents);

            if (cf_aabb_to_aabb(bullet_aabb, enemy_aabb)) {
                // Damage the enemy
                enemy->health.current -= 1;

                // Destroy bullet
                bullet->is_alive = false;

                // If enemy survives, push it upwards and spawn particles
                if (enemy->health.current > 0) {
                    enemy->position.y += 5.0f;  // Push upwards by 5 pixels
                    screenshake_add(&g_state->screenshake, 0.5f);
                    play_sound(SOUND_HIT);
                } else {
                    g_state->score += enemy->score;
                    // Destroy enemy
                    enemy->is_alive = false;

                    spawn_explosion(make_explosion(enemy->position));
                    spawn_explosion_particle_burst(enemy->position, COLOR_SOURCE_ENEMY(enemy->type));
                    spawn_floating_score(make_floating_score(enemy->position, enemy->score));
                    screenshake_add(&g_state->screenshake, 1.0f);
                    play_sound(SOUND_EXPLOSION);
                }

                // Get bullet direction from velocity and reverse it
                auto bullet_dir = cf_mul(cf_norm(bullet->velocity), -1.0f);

                // Spawn white debris particles opposite to the bullet's direction
                spawn_hit_particle_burst(5, enemy->position, bullet_dir);

                // Bullet is destroyed, no need to check against more enemies
                break;
            }
        }
    }
}

static void player_vs_threats(
    const Player* restrict player,
    const size_t enemies_count,
    Enemy        enemies[static restrict enemies_count],
    const size_t enemy_bullets_count,
    EnemyBullet  enemy_bullets[static restrict enemy_bullets_count]
) {
    if (enemies_count == 0 && enemy_bullets_count == 0) { return; }
    if (!player->is_alive || player->is_invincible) { return; }

    auto player_aabb = cf_make_aabb_center_half_extents(player->position, player->collider.half_extents);

    // Check collisions with enemies
    for (size_t i = 0; i < enemies_count; ++i) {
        if (!enemies[i].is_alive) { continue; }

        auto enemy      = &enemies[i];
        auto enemy_aabb = cf_make_aabb_center_half_extents(enemy->position, enemy->collider.half_extents);

        if (cf_aabb_to_aabb(player_aabb, enemy_aabb)) {
            enemy->is_alive = false;
            damage_player();
            return;  // Player is dead, no need to check more collisions
        }
    }

    // Check collisions with enemy bullets
    for (size_t i = 0; i < enemy_bullets_count; ++i) {
        if (!enemy_bullets[i].is_alive) { continue; }

        auto enemy_bullet = &enemy_bullets[i];
        auto enemy_aabb = cf_make_aabb_center_half_extents(enemy_bullet->position, enemy_bullet->collider.half_extents);

        if (cf_aabb_to_aabb(player_aabb, enemy_aabb)) {
            enemy_bullet->is_alive = false;
            damage_player();
            return;  // Player is dead, no need to check more collisions
        }
    }
}

void update_collision(void) {
    player_bullets_vs_enemies(
        g_state->player_bullets_count, g_state->player_bullets, g_state->enemies_count, g_state->enemies
    );
    player_vs_threats(
        &g_state->player, g_state->enemies_count, g_state->enemies, g_state->enemy_bullets_count, g_state->enemy_bullets
    );
}
