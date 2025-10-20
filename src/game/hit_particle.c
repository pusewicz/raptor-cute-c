#include "hit_particle.h"

#include <cute_c_runtime.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <stddef.h>
#include <string.h>

#include "../engine/game_state.h"
#include "component.h"
#include "cute_time.h"

HitParticle make_hit_particle(float x, float y, CF_V2 direction) {
    // Calculate the base angle from the direction vector
    float base_angle     = CF_ATAN2F(direction.y, direction.x);
    float spread         = cf_rnd_range_float(&g_state->rnd, -0.5f, 0.5f);  // ±0.5 radians spread
    float angle          = base_angle + spread;
    float speed          = cf_rnd_range_float(&g_state->rnd, 0.5f, 2.0f);

    HitParticle particle = (HitParticle){
        .is_alive   = true,
        .position   = cf_v2(x, y),
        .velocity   = cf_v2(CF_COSF(angle) * speed, CF_SINF(angle) * speed),
        .lifetime   = cf_rnd_range_float(&g_state->rnd, 0.5f, 0.85f),
        .time_alive = 0.0f,
        .size       = (float)cf_rnd_range_int(&g_state->rnd, 1, 2),
        // Use the shared particle sprite (no allocation needed)
        .sprite     = g_state->sprites.particle,
    };

    return particle;
}

void spawn_hit_particle(HitParticle particle) {
    CF_ASSERT(g_state->hit_particles);
    CF_ASSERT(g_state->hit_particles_count < g_state->hit_particles_capacity);

    g_state->hit_particles[g_state->hit_particles_count++] = particle;
}

void spawn_hit_particles(size_t count, const HitParticle particles[static restrict count]) {
    CF_ASSERT(g_state->hit_particles_count + count <= g_state->hit_particles_capacity);

    CF_MEMCPY(&g_state->hit_particles[g_state->hit_particles_count], particles, count * sizeof(*particles));

    g_state->hit_particles_count += count;
}

void spawn_hit_particle_burst(size_t count, CF_V2 pos, CF_V2 dir) {
    HitParticle burst[count];

    for (size_t i = 0; i < count; ++i) { burst[i] = make_hit_particle(pos.x, pos.y, dir); }

    spawn_hit_particles(count, burst);
}

void cleanup_hit_particles() {
    size_t write_idx = 0;

    for (size_t i = 0; i < g_state->hit_particles_count; i++) {
        if (g_state->hit_particles[i].is_alive) { g_state->hit_particles[write_idx++] = g_state->hit_particles[i]; }
    }

    g_state->hit_particles_count = write_idx;
}

void update_particles() {
    for (size_t i = 0; i < g_state->hit_particles_count; ++i) {
        auto particle = &g_state->hit_particles[i];

        if (!particle->is_alive) { continue; }

        // Update particle lifetime
        particle->time_alive += CF_DELTA_TIME;

        // Destroy particle if lifetime exceeded
        if (particle->time_alive >= particle->lifetime) {
            particle->is_alive = false;
            continue;
        }

        // Calculate fade based on lifetime and update sprite opacity (fade to 50%, not 0%)
        particle->sprite.opacity = 1.0f - (particle->time_alive / particle->lifetime) * 0.5f;

        cf_sprite_update(&particle->sprite);
    }
}

// Parallax particle constants
#define PARALLAX_LAYER_COUNT 4
static const float LAYER_SPEEDS[PARALLAX_LAYER_COUNT]   = {8.0f, 12.0f, 16.0f, 24.0f};
static const float LAYER_SIZES[PARALLAX_LAYER_COUNT]    = {1.0f, 1.5f, 2.0f, 2.5f};
static const int   LAYER_ZINDICES[PARALLAX_LAYER_COUNT] = {Z_PARALLAX, Z_PARALLAX, Z_PARALLAX, Z_PARALLAX};
static const int   PARTICLES_PER_LAYER                  = 4;

void init_parallax_particles(void) {
    CF_ASSERT(g_state->parallax_particles);

    const float canvas_width  = g_state->canvas_size.x;
    const float canvas_height = g_state->canvas_size.y;

    for (int layer = 0; layer < PARALLAX_LAYER_COUNT; ++layer) {
        for (int i = 0; i < PARTICLES_PER_LAYER; ++i) {
            // Random position across the screen
            float x              = cf_rnd_range_float(&g_state->rnd, -canvas_width / 2, canvas_width / 2);
            float y              = cf_rnd_range_float(&g_state->rnd, -canvas_height / 2, canvas_height / 2);

            HitParticle particle = (HitParticle){
                .is_alive   = true,
                .position   = cf_v2(x, y),
                .velocity   = cf_v2(0.0f, -LAYER_SPEEDS[layer]),  // Move downward
                .lifetime   = 999999.0f,                          // Never expire
                .time_alive = 0.0f,
                .size       = LAYER_SIZES[layer],
                .sprite     = g_state->sprites.particle,
                .z_index    = LAYER_ZINDICES[layer],
            };

            // Store layer index in time_alive for later use (hack to reuse HitParticle)
            particle.time_alive                                              = (float)layer;

            g_state->parallax_particles[g_state->parallax_particles_count++] = particle;
        }
    }
}

void update_parallax_particles(void) {
    const float canvas_width  = g_state->canvas_size.x;
    const float canvas_height = g_state->canvas_size.y;
    const float player_x      = g_state->player.position.x;

    for (size_t i = 0; i < g_state->parallax_particles_count; ++i) {
        auto particle         = &g_state->parallax_particles[i];

        // Get layer index from time_alive (stored during init)
        int   layer           = (int)particle->time_alive;
        float parallax_scale  = (float)(layer + 1) / PARALLAX_LAYER_COUNT;

        // Apply horizontal parallax based on player position
        float parallax_offset = -player_x * parallax_scale * 0.15f;

        // Update vertical position
        particle->position.y += particle->velocity.y * CF_DELTA_TIME;

        // Wrap around when going off the bottom
        if (particle->position.y < -canvas_height / 2 - 10.0f) {
            particle->position.y = canvas_height / 2 + 10.0f;
            particle->position.x = cf_rnd_range_float(&g_state->rnd, -canvas_width / 2, canvas_width / 2);
        }

        // Store the parallax offset for rendering (reuse velocity.x as storage)
        particle->velocity.x = parallax_offset;

        cf_sprite_update(&particle->sprite);
    }
}
