#pragma once

#include <cute_color.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "../game/enemy.h"

#define COLOR_SOURCE_PLAYER()  ((ColorSource){.type = COLOR_SOURCE_TYPE_PLAYER})
#define COLOR_SOURCE_ENEMY(et) ((ColorSource){.type = COLOR_SOURCE_TYPE_ENEMY, .data.enemy_type = (et)})

typedef enum {
    COLOR_SOURCE_TYPE_PLAYER,
    COLOR_SOURCE_TYPE_ENEMY,
} ColorSourceType;

typedef struct ColorSource {
    ColorSourceType type;
    union {
        EnemyType enemy_type;
    } data;
} ColorSource;

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

ExplosionParticle make_explosion_particle(CF_V2 position, CF_Color color, float angle);
void              spawn_explosion_particle(ExplosionParticle particle);
void              spawn_explosion_particles(size_t count, const ExplosionParticle particles[static restrict count]);
void              spawn_explosion_particle_burst(CF_V2 pos, const ColorSource color_source);
void              cleanup_explosion_particles(void);
void              update_explosion_particles(void);
void              render_explosion_particles(void);
