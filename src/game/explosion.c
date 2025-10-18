#include "explosion.h"

#include <cute_c_runtime.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "asset/sprite.h"
#include "component.h"

Explosion make_explosion(float x, float y) {
    Explosion explosion = (Explosion){.position = cf_v2(x, y), .z_index = Z_SPRITES, .is_alive = true};

    // Sprite
    load_sprite(&explosion.sprite, "assets/explosion.ase");
    cf_sprite_set_loop(&explosion.sprite, false);

    return explosion;
}

void spawn_explosion(Explosion explosion) {
    CF_ASSERT(g_state->explosions);
    CF_ASSERT(g_state->explosions_count < g_state->explosions_capacity - 1);
    g_state->explosions[g_state->explosions_count++] = explosion;
}

void cleanup_explosions(void) {
    // Mark finished explosions as dead
    for (size_t i = 0; i < g_state->explosions_count; ++i) {
        auto explosion = &g_state->explosions[i];

        if (!explosion->is_alive) continue;

        if (!cf_sprite_get_loop(&explosion->sprite) && cf_sprite_will_finish(&explosion->sprite)) {
            explosion->is_alive = false;
        }
    }

    size_t write_idx = 0;
    // Cleanup explosions array
    for (size_t i = 0; i < g_state->explosions_count; ++i) {
        if (g_state->explosions[i].is_alive) { g_state->explosions[write_idx++] = g_state->explosions[i]; }
    }
    g_state->explosions_count = write_idx;
}
