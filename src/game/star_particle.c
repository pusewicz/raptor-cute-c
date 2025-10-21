#include "star_particle.h"

#include <cute_c_runtime.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "component.h"

// Star particle constants
constexpr int      STAR_LAYER_COUNT                 = 4;
constexpr int      PARTICLES_PER_LAYER              = 4;
static const float LAYER_SPEEDS[STAR_LAYER_COUNT]   = {8.0f, 12.0f, 16.0f, 24.0f};
static const float LAYER_SIZES[STAR_LAYER_COUNT]    = {1.0f, 1.5f, 2.0f, 2.5f};
static const int   LAYER_ZINDICES[STAR_LAYER_COUNT] = {Z_PARALLAX, Z_PARALLAX, Z_PARALLAX, Z_PARALLAX};

void init_star_particles(void) {
    CF_ASSERT(g_state->star_particles);
    CF_ASSERT(g_state->star_particles_count + PARTICLES_PER_LAYER <= g_state->star_particles_capacity);

    const float canvas_width  = g_state->canvas_size.x;
    const float canvas_height = g_state->canvas_size.y;

    for (int layer = 0; layer < STAR_LAYER_COUNT; ++layer) {
        for (int i = 0; i < PARTICLES_PER_LAYER; ++i) {
            // Random position across the screen
            float x               = cf_rnd_range_float(&g_state->rnd, -canvas_width / 2, canvas_width / 2);
            float y               = cf_rnd_range_float(&g_state->rnd, -canvas_height / 2, canvas_height / 2);

            StarParticle particle = (StarParticle){
                .position        = cf_v2(x, y),
                .velocity        = cf_v2(0.0f, -LAYER_SPEEDS[layer]),  // Move downward
                .size            = LAYER_SIZES[layer],
                .sprite          = g_state->sprites.particle,
                .z_index         = LAYER_ZINDICES[layer],
                .layer           = layer,
                .parallax_offset = 0.0f,
            };

            g_state->star_particles[g_state->star_particles_count++] = particle;
        }
    }
}

void update_star_particles(void) {
    const float canvas_width  = g_state->canvas_size.x;
    const float canvas_height = g_state->canvas_size.y;
    const float player_x      = g_state->player.position.x;

    for (size_t i = 0; i < g_state->star_particles_count; ++i) {
        auto particle             = &g_state->star_particles[i];

        float parallax_scale      = (float)(particle->layer + 1) / STAR_LAYER_COUNT;

        // Apply horizontal parallax based on player position
        particle->parallax_offset = -player_x * parallax_scale * 0.05f;

        // Update vertical position
        particle->position.y += particle->velocity.y * CF_DELTA_TIME;

        // Wrap around when going off the bottom
        if (particle->position.y < -canvas_height / 2 - 10.0f) {
            particle->position.y = canvas_height / 2 + 10.0f;
            particle->position.x = cf_rnd_range_float(&g_state->rnd, -canvas_width / 2, canvas_width / 2);
        }

        cf_sprite_update(&particle->sprite);
    }
}
