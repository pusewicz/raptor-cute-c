#include "explosion_particle.h"

#include <cute_c_runtime.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <stddef.h>
#include <string.h>

#include "../engine/game_state.h"

// Helper function to sample colors from sprite
// For now, we'll sample from predefined color palettes based on sprite frame
static CF_Color sample_sprite_color(const CF_Sprite* sprite) {
    // Get the current frame's pixel data
    // Note: This is a simplified approach - we'd ideally sample actual pixels
    // but for now we'll use predefined colors that match the enemy sprites

    // Sample some common colors from the sprite
    // These are typical colors found in retro shooter sprites
    CF_Color colors[] = {
        cf_make_color_rgb(255, 100, 100),  // Red
        cf_make_color_rgb(255, 150, 50),   // Orange
        cf_make_color_rgb(255, 200, 100),  // Yellow
        cf_make_color_rgb(100, 200, 255),  // Light blue
        cf_make_color_rgb(150, 150, 255),  // Purple
        cf_make_color_rgb(200, 200, 200),  // White/gray
    };

    int index = cf_rnd_range_int(&g_state->rnd, 0, (int)(sizeof(colors) / sizeof(colors[0]) - 1));
    return colors[index];
}

ExplosionParticle make_explosion_particle(float x, float y, CF_Color color, float angle) {
    float speed = cf_rnd_range_float(&g_state->rnd, 1.0f, 3.0f);

    ExplosionParticle particle = (ExplosionParticle){
        .is_alive   = true,
        .position   = cf_v2(x, y),
        .velocity   = cf_v2(CF_COSF(angle) * speed, CF_SINF(angle) * speed),
        .lifetime   = cf_rnd_range_float(&g_state->rnd, 0.5f, 1.2f),
        .time_alive = 0.0f,
        .size       = (float)cf_rnd_range_int(&g_state->rnd, 1, 3),
        .color      = color,
        // Use the shared particle sprite (no allocation needed)
        .sprite     = g_state->sprites.particle,
    };

    return particle;
}

void spawn_explosion_particle(ExplosionParticle particle) {
    CF_ASSERT(g_state->explosion_particles);
    CF_ASSERT(g_state->explosion_particles_count < g_state->explosion_particles_capacity);

    g_state->explosion_particles[g_state->explosion_particles_count++] = particle;
}

void spawn_explosion_particles(size_t count, const ExplosionParticle particles[static restrict count]) {
    CF_ASSERT(g_state->explosion_particles_count + count <= g_state->explosion_particles_capacity);

    CF_MEMCPY(&g_state->explosion_particles[g_state->explosion_particles_count], particles, count * sizeof(*particles));

    g_state->explosion_particles_count += count;
}

void spawn_explosion_particle_burst(CF_V2 pos, const CF_Sprite* sprite) {
    // Create radial burst of particles
    const size_t particle_count = 20;  // Number of particles in explosion
    ExplosionParticle burst[particle_count];

    for (size_t i = 0; i < particle_count; ++i) {
        // Calculate angle for radial dispersion (360 degrees)
        float angle = (float)i / (float)particle_count * CF_PI * 2.0f;

        // Sample color from sprite
        CF_Color color = sample_sprite_color(sprite);

        burst[i] = make_explosion_particle(pos.x, pos.y, color, angle);
    }

    spawn_explosion_particles(particle_count, burst);
}

void cleanup_explosion_particles() {
    size_t write_idx = 0;

    for (size_t i = 0; i < g_state->explosion_particles_count; i++) {
        if (g_state->explosion_particles[i].is_alive) {
            g_state->explosion_particles[write_idx++] = g_state->explosion_particles[i];
        }
    }

    g_state->explosion_particles_count = write_idx;
}

void update_explosion_particles() {
    for (size_t i = 0; i < g_state->explosion_particles_count; ++i) {
        auto particle = &g_state->explosion_particles[i];

        if (!particle->is_alive) { continue; }

        // Update particle lifetime
        particle->time_alive += CF_DELTA_TIME;

        // Destroy particle if lifetime exceeded
        if (particle->time_alive >= particle->lifetime) {
            particle->is_alive = false;
            continue;
        }

        // Update position based on velocity
        particle->position = cf_add_v2(particle->position, cf_mul_v2_f(particle->velocity, CF_DELTA_TIME));

        // Calculate fade based on lifetime and update sprite opacity
        particle->sprite.opacity = 1.0f - (particle->time_alive / particle->lifetime);

        cf_sprite_update(&particle->sprite);
    }
}
