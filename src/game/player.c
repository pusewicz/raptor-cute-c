#include "player.h"

#include <cute_c_runtime.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <stddef.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "asset/audio.h"
#include "asset/sprite.h"
#include "component.h"
#include "explosion.h"
#include "player_bullet.h"

constexpr float WEAPON_DEFAULT_COOLDOWN = 0.15f;  // Time needed to let the player shoot again

Player make_player(float x, float y) {
    Player player                  = {0};
    player.is_alive                = true;
    player.is_invincible           = false;
    player.invincibility_timer     = 0.0f;
    player.respawn_delay           = 0.0f;

    // Position
    player.position.x              = x;
    player.position.y              = y;

    // Velocity
    player.velocity.x              = 0.0f;
    player.velocity.y              = 0.0f;

    // Input
    player.input.up                = false;
    player.input.down              = false;
    player.input.left              = false;
    player.input.right             = false;

    // Sprites
    player.sprite                  = get_sprite(SPRITE_PLAYER);  // TODO: Should this not store sprites?
    player.booster_sprite          = get_sprite(SPRITE_BOOSTERS);
    player.booster_sprite.offset.y = -player.sprite.h;
    player.z_index                 = Z_PLAYER_SPRITE;
    cf_sprite_play(&player.sprite, "default");
    cf_sprite_play(&player.booster_sprite, "default");

    // Collider
    player.collider.half_extents  = cf_v2(player.sprite.w / 4.0, player.sprite.h / 4.0);

    // Weapon
    player.weapon.cooldown        = WEAPON_DEFAULT_COOLDOWN;
    player.weapon.time_since_shot = WEAPON_DEFAULT_COOLDOWN;

    return player;
}

void damage_player(void) {
    auto player = &g_state->player;

    // Only damage if player is alive and not invincible
    if (!player->is_alive || player->is_invincible) { return; }

    // Decrement lives
    g_state->lives--;

    // Create explosion at player position
    spawn_explosion(make_explosion(player->position.x, player->position.y));
    play_sound(SOUND_EXPLOSION);
    play_sound(SOUND_DEATH);

    // Mark player as dead
    player->is_alive      = false;
    player->is_invincible = false;

    // Set respawn delay if player has lives remaining
    if (g_state->lives > 0) {
        player->respawn_delay = 2.0f;  // 2 second respawn delay
    } else {
        // Game over
        g_state->is_game_over = true;
        play_sound(SOUND_GAME_OVER);
    }
}

void update_player(Player* player) {
    // Handle respawn delay
    if (!player->is_alive && player->respawn_delay > 0.0f) {
        player->respawn_delay -= CF_DELTA_TIME;

        if (player->respawn_delay <= 0.0f) {
            // Respawn player
            player->is_alive            = true;
            player->is_invincible       = true;
            player->invincibility_timer = 3.0f;  // 3 seconds of invincibility

            // Reset player position
            player->position.x          = 0.0f;
            player->position.y          = -g_state->canvas_size.y / 3;

            play_sound(SOUND_REVEAL);
        }

        return;
    }

    // Handle invincibility timer
    if (player->is_invincible && player->invincibility_timer > 0.0f) {
        player->invincibility_timer -= CF_DELTA_TIME;
        if (player->invincibility_timer <= 0.0f) { player->is_invincible = false; }
    }

    // Handle input
    float speed        = 1.0f;
    player->velocity.x = player->velocity.y = 0.0f;

    if (player->input.up) player->velocity.y += speed;
    if (player->input.down) player->velocity.y -= speed;
    if (player->input.left) player->velocity.x -= speed;
    if (player->input.right) player->velocity.x += speed;

    // Handle shooting
    if (player->weapon.time_since_shot < player->weapon.cooldown) {
        player->weapon.time_since_shot += CF_DELTA_TIME;
    } else if (player->input.shoot) {
        player->weapon.time_since_shot = 0.0f;

        spawn_player_bullet(make_player_bullet(player->position.x, player->position.y, cf_v2(0, 1)));

        play_sound(SOUND_LASER);
    }
}

void render_player(Player* player) {
    if (!player->is_alive) { return; }

    if (player->velocity.x > 0) {
        if (!cf_sprite_is_playing(&player->sprite, "right")) {
            cf_sprite_play(&player->sprite, "right");
            cf_sprite_play(&player->booster_sprite, "right");
        }
    } else if (player->velocity.x < 0) {
        if (!cf_sprite_is_playing(&player->sprite, "left")) {
            cf_sprite_play(&player->sprite, "left");
            cf_sprite_play(&player->booster_sprite, "left");
        }
    } else if (!cf_sprite_is_playing(&player->sprite, "default")) {
        cf_sprite_play(&player->sprite, "default");
        cf_sprite_play(&player->booster_sprite, "default");
    }

    cf_sprite_update(&player->sprite);
    cf_sprite_update(&player->booster_sprite);

    // Flicker effect during invincibility
    bool should_render = true;
    if (player->is_invincible) {
        // Flicker every 0.1 seconds
        should_render = ((int)(player->invincibility_timer * 10) % 2) == 0;
    }

    if (should_render) {
        cf_draw() {
            cf_draw_layer(player->z_index) {
                cf_draw_translate_v2(player->position);
                cf_draw_sprite(&player->sprite);
                cf_draw_sprite(&player->booster_sprite);
            }
        }
    }
}
