#pragma once

#include <cute_math.h>
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
  bool shoot;
} input_t;

typedef struct weapon_t {
  float cooldown;           // Time between shots in seconds
  float time_since_shot;    // Time since last shot in seconds
} weapon_t;

typedef struct bullet_t {
  CF_V2 direction;
} bullet_t;

/*
 * Components
 */
typedef struct {
  ecs_id_t position;
  ecs_id_t velocity;
  ecs_id_t input;
  ecs_id_t sprite;
  ecs_id_t bullet;
  ecs_id_t weapon;
} Components;

/*
 * Systems
 */
typedef struct {
  ecs_id_t input;
  ecs_id_t weapon;
  ecs_id_t movement;
  ecs_id_t render;
} Systems;

void register_components(GameState *state);
void register_systems(GameState *state);
void update_system_callbacks(GameState *state);
void update_systems(GameState *state);
