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

/*
 * Component IDs
 */
extern ecs_id_t PosComp;       // Position component
extern ecs_id_t VelComp;       // Velocity component
extern ecs_id_t InputComp;     // Input component
extern ecs_id_t SpriteComp;    // Sprite component

void register_components(GameState *state);
void register_systems(GameState *state);
void update_systems(GameState *state);
