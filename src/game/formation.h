#pragma once

#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "enemy.h"

typedef struct {
    float x_offset;
    float y_offset;
    float spawn_delay;
} FormationPoint;

typedef struct {
    const char*           name;
    const FormationPoint* points;
    size_t                points_count;
} Formation;

// Formation library - predefined formations
extern const Formation FORMATION_LINE_HORIZONTAL;
extern const Formation FORMATION_LINE_VERTICAL;
extern const Formation FORMATION_DIAMOND;
extern const Formation FORMATION_ARROW_DOWN;
extern const Formation FORMATION_V_SHAPE;
extern const Formation FORMATION_WAVE;

// Spawner functions
void formation_spawn(const Formation* formation, CF_V2 origin, EnemyType enemy_type);
void formation_spawn_with_shoot_chance(
    const Formation* formation,
    CF_V2            origin,
    EnemyType        enemy_type,
    float            shoot_chance
);
