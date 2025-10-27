#pragma once

#include <cute_color.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

typedef enum EnemyType EnemyType;

typedef struct ExplosionParticle {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    CF_Color  color;       // Sampled color from enemy sprite
    float     lifetime;    // Total lifetime in seconds
    float     time_alive;  // Time alive in seconds
    float     size;        // Particle size scale
    bool      is_alive;
} ExplosionParticle;

ExplosionParticle make_explosion_particle(float x, float y, CF_Color color, float angle);
void              spawn_explosion_particle(ExplosionParticle particle);
void              spawn_explosion_particles(size_t count, const ExplosionParticle particles[static restrict count]);
void              spawn_explosion_particle_burst(CF_V2 pos, const EnemyType enemy_type);
void              cleanup_explosion_particles(void);
void              update_explosion_particles(void);
void              render_explosion_particles(void);
