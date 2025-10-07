#pragma once

#include <cute_math.h>
#include <pico_ecs.h>

typedef enum EnemyType {
    ENEMY_TYPE_ALAN,
    ENEMY_TYPE_BON_BON,
    ENEMY_TYPE_LIPS,
} EnemyType;

ecs_id_t make_background_scroll(void);
ecs_id_t make_player_bullet(float x, float y, CF_V2 direction);
ecs_id_t make_enemy_bullet(float x, float y, CF_V2 direction);
ecs_id_t make_enemy(float x, float y);
ecs_id_t make_enemy_of_type(float x, float y, EnemyType type);
ecs_id_t make_player(float x, float y);
ecs_id_t make_explosion(float x, float y);
ecs_id_t make_shooting_star(float y);
void     make_hit_particles(float x, float y, CF_V2 direction, int count);
