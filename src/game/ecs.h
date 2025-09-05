#pragma once

#include <pico_ecs.h>

typedef struct GameState GameState;

/*
 * Component structures
 */
typedef struct input_t {
  bool up;
  bool down;
  bool left;
  bool right;
} input_t;

void register_components(GameState *state);
void register_systems(GameState *state);
void update_system_callbacks(GameState *state);
void update_systems(GameState *state);
