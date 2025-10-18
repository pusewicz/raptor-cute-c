#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

typedef struct HitParticle {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    float     lifetime;    // Total lifetime in seconds
    float     time_alive;  // Time alive in seconds
    float     size;        // Particle size scale
    bool      is_alive;
} HitParticle;

HitParticle make_hit_particle(float x, float y, CF_V2 direction);
void        spawn_hit_particle(HitParticle hit_particle);
void        spawn_hit_particles(const HitParticle* particles, int count);
void        cleanup_hit_particles(void);
void        update_particles(void);
