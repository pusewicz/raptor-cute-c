#include "hit_particle.h"

#include <cute_c_runtime.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <stddef.h>
#include <string.h>

#include "../engine/game_state.h"

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
