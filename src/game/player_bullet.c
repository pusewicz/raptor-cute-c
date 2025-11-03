#include "player_bullet.h"

#include <cute_c_runtime.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "asset/sprite.h"
#include "component.h"

constexpr float PLAYER_BULLET_DEFAULT_SPEED = 3.0f;

PlayerBullet make_player_bullet(CF_V2 position, CF_V2 direction) {
    PlayerBullet bullet = (PlayerBullet){
        .is_alive = true,
        .position = position,
    };

    // Velocity
    bullet.velocity.x            = 0.0f;
    bullet.velocity.y            = PLAYER_BULLET_DEFAULT_SPEED * direction.y;

    // Sprite
    bullet.sprite                = get_sprite(SPRITE_BULLET);
    bullet.z_index               = Z_SPRITES;

    // Collider
    bullet.collider.half_extents = cf_v2(bullet.sprite.w / 4.2, bullet.sprite.h / 4.2);

    return bullet;
}

void spawn_player_bullet(PlayerBullet player_bullet) {
    CF_ASSERT(g_state->player_bullets);
    CF_ASSERT(g_state->player_bullets_count < g_state->player_bullets_capacity);
    g_state->player_bullets[g_state->player_bullets_count++] = player_bullet;
}

void cleanup_player_bullets() {
    int write_idx = 0;
    for (size_t i = 0; i < g_state->player_bullets_count; i++) {
        if (g_state->player_bullets[i].is_alive) { g_state->player_bullets[write_idx++] = g_state->player_bullets[i]; }
    }
    g_state->player_bullets_count = write_idx;
}
