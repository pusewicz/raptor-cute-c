#pragma once

#include "../engine/game_state.h"

#include <pico_ecs.h>

ecs_id_t make_player(GameState *state, float x, float y);
