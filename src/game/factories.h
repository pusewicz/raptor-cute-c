#pragma once

#include <cute_math.h>
#include <pico_ecs.h>

ecs_id_t make_background_scroll(void);
ecs_id_t make_bullet(float x, float y, CF_V2 direction);
ecs_id_t make_enemy(float x, float y);
ecs_id_t make_player(float x, float y);
