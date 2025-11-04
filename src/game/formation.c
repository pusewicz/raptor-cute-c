#include "formation.h"

#include <cute_math.h>
#include <cute_time.h>
#include <stddef.h>

#include "../engine/common.h"
#include "enemy.h"

// Define the actual formations
static const FormationPoint line_horizontal_points[] = {
    {-40, 0, 0.0f},
    {-20, 0, 0.0f},
    {  0, 0, 0.0f},
    { 20, 0, 0.0f},
    { 40, 0, 0.0f},
};

static const FormationPoint line_vertical_points[] = {
    {0,  30, 0.0f},
    {0,  15, 0.0f},
    {0,   0, 0.0f},
    {0, -15, 0.0f},
    {0, -30, 0.0f},
};

static const FormationPoint diamond_points[] = {
    {  0,  48, 0.0f}, // top
    {-24,  24, 0.0f}, // top-left
    { 24,  24, 0.0f}, // top-right
    {-48,   0, 0.0f}, // left
    { 48,   0, 0.0f}, // right
    {-24, -24, 0.0f}, // bottom-left
    { 24, -24, 0.0f}, // bottom-right
    {  0, -48, 0.0f}, // bottom
};

static const FormationPoint arrow_down_points[] = {
    {  0,  36, 0.0f}, // tip
    {-12,  24, 0.0f},
    { 12,  24, 0.0f},
    {-24,  12, 0.0f},
    { 24,  12, 0.0f},
    {-36,   0, 0.0f},
    { 36,   0, 0.0f},
    {-48, -12, 0.0f},
    { 48, -12, 0.0f},
};

static const FormationPoint v_shape_points[] = {
    {  0,   0, 0.0f}, // center lead
    {-15, -10, 0.0f},
    { 15, -10, 0.0f},
    {-30, -20, 0.0f},
    { 30, -20, 0.0f},
    {-45, -30, 0.0f},
    { 45, -30, 0.0f},
};

static const FormationPoint wave_points[] = {
    {-60,   0, 0.0f},
    {-45,  15, 0.0f},
    {-30,   0, 0.0f},
    {-15, -15, 0.0f},
    {  0,   0, 0.0f},
    { 15,  15, 0.0f},
    { 30,   0, 0.0f},
    { 45, -15, 0.0f},
    { 60,   0, 0.0f},
};

// Export the formations
const Formation FORMATION_LINE_HORIZONTAL = {
    .name         = "line_horizontal",
    .points       = line_horizontal_points,
    .points_count = countof(line_horizontal_points),
};

const Formation FORMATION_LINE_VERTICAL = {
    .name         = "line_vertical",
    .points       = line_vertical_points,
    .points_count = countof(line_vertical_points),
};

const Formation FORMATION_DIAMOND = {
    .name         = "diamond",
    .points       = diamond_points,
    .points_count = countof(diamond_points),
};

const Formation FORMATION_ARROW_DOWN = {
    .name         = "arrow_down",
    .points       = arrow_down_points,
    .points_count = countof(arrow_down_points),
};

const Formation FORMATION_V_SHAPE = {
    .name         = "v_shape",
    .points       = v_shape_points,
    .points_count = countof(v_shape_points),
};

const Formation FORMATION_WAVE = {
    .name         = "wave",
    .points       = wave_points,
    .points_count = countof(wave_points),
};

// Spawner implementation
void formation_spawn(const Formation* formation, CF_V2 origin, EnemyType enemy_type) {
    for (size_t i = 0; i < formation->points_count; ++i) {
        const FormationPoint* point     = &formation->points[i];
        CF_V2                 world_pos = {origin.x + point->x_offset, origin.y + point->y_offset};
        auto                  enemy     = make_enemy_of_type(world_pos, enemy_type);

        spawn_enemy(enemy);
    }
}

void formation_spawn_with_shoot_chance(
    const Formation* formation, CF_V2 origin, EnemyType enemy_type, float shoot_chance
) {
    for (size_t i = 0; i < formation->points_count; ++i) {
        const FormationPoint* point     = &formation->points[i];
        CF_V2                 world_pos = {origin.x + point->x_offset, origin.y + point->y_offset};
        auto                  enemy     = make_enemy_of_type(world_pos, enemy_type);

        set_enemy_shoot_chance(&enemy, shoot_chance);
        spawn_enemy(enemy);
    }
}
