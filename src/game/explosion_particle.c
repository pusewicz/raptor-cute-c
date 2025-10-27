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

#include "../engine/common.h"
#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "../engine/log.h"
#include "component.h"
#include "enemy.h"
#include "movement.h"

static CF_Color sample_sprite_color(const EnemyType enemy_type) {
    CF_Color colors[ENEMY_TYPE_COUNT][3] = {
        [ENEMY_TYPE_ALAN] =
            {
                               cf_make_color_hex(0x77cc2a),
                               cf_make_color_hex(0x077d53),
                               cf_make_color_hex(0xffc41f),
                               },
        [ENEMY_TYPE_BON_BON] =
            {
                               cf_make_color_hex(0xffc41f),
                               cf_make_color_hex(0xe67300),
                               cf_make_color_hex(0xf2f1f0),
                               },
        [ENEMY_TYPE_LIPS] = {
                               cf_make_color_hex(0xea58ad),
                               cf_make_color_hex(0x9e1328),
                               cf_make_color_hex(0xffacbf),
                               }
    };

    int      index = cf_rnd_range_int(&g_state->rnd, 0, lengthof(colors[enemy_type]));
    CF_Color color = colors[enemy_type][index];

    return color;
}

ExplosionParticle make_explosion_particle(float x, float y, CF_Color color, float angle) {
    float speed                = cf_rnd_range_float(&g_state->rnd, 0.5f, 1.0f);

    ExplosionParticle particle = (ExplosionParticle){
        .is_alive   = true,
        .position   = cf_v2(x, y),
        .velocity   = cf_v2(CF_COSF(angle) * speed, CF_SINF(angle) * speed),
        .lifetime   = cf_rnd_range_float(&g_state->rnd, 0.5f, 0.8f),
        .time_alive = 0.0f,
        .size       = (float)cf_rnd_range_int(&g_state->rnd, 1, 2),
        .color      = color,
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

void spawn_explosion_particle_burst(CF_V2 pos, const EnemyType enemy_type) {
    // Create radial burst of particles
    constexpr size_t  particle_count = 10;  // Number of particles in explosion
    ExplosionParticle burst[particle_count];

    for (size_t i = 0; i < particle_count; ++i) {
        // Calculate angle for radial dispersion (360 degrees)
        float angle    = (float)i / (float)particle_count * CF_PI * 2.0f;

        // Sample color from sprite
        CF_Color color = sample_sprite_color(enemy_type);

        burst[i]       = make_explosion_particle(pos.x, pos.y, color, angle);
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

        update_movement(&particle->position, &particle->velocity);

        // Calculate fade based on lifetime and update sprite opacity
        particle->sprite.opacity = 1.0f - (particle->time_alive / particle->lifetime);

        cf_sprite_update(&particle->sprite);
    }
}

void render_explosion_particles() {
    // Render explosion particles with colors
    cf_draw_push_shader(g_state->recolor);
    for (size_t i = 0; i < g_state->explosion_particles_count; i++) {
        cf_draw() {
            cf_draw_layer(Z_PARTICLES) {
                cf_draw_translate_v2(g_state->explosion_particles[i].position);
                cf_draw_scale(g_state->explosion_particles[i].size, g_state->explosion_particles[i].size);
                // Apply color to the sprite
                cf_draw_push_vertex_attributes(
                    g_state->explosion_particles[i].color.r,
                    g_state->explosion_particles[i].color.g,
                    g_state->explosion_particles[i].color.b,
                    1.0f
                );
                cf_draw_sprite(&g_state->explosion_particles[i].sprite);
                cf_draw_pop_vertex_attributes();
            }
        }
    }
    cf_draw_pop_shader();
}
