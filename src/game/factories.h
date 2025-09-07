#pragma once

#include "../engine/game_state.h"
#include "ecs.h"

#include <pico_ecs.h>

typedef struct CF_V2 CF_V2;

ecs_id_t make_player(GameState *state, float x, float y);
ecs_id_t make_bullet(GameState *state, float x, float y, CF_V2 direction);
