#pragma once

#include <cute_math.h>
#include <pico_ecs.h>
#include <stdbool.h>

/*
 * Component structures
 */

typedef struct BulletComponent {
  CF_V2 direction;
} BulletComponent;

typedef struct InputComponent {
  bool up;
  bool down;
  bool left;
  bool right;
  bool shoot;
} InputComponent;

typedef struct EnemySpawnComponent {
  float spawn_interval;           // Time between spawns in seconds
  float time_since_last_spawn;    // Time since last spawn in seconds
  int   max_enemies;              // Maximum number of enemies allowed
  int   current_enemy_count;      // Current number of enemies spawned
} EnemySpawnComponent;

typedef struct WeaponComponent {
  float cooldown;           // Time between shots in seconds
  float time_since_shot;    // Time since last shot in seconds
} WeaponComponent;

/*
 * Components
 */
typedef struct {
  ecs_id_t bullet;
  ecs_id_t enemy_spawn;
  ecs_id_t input;
  ecs_id_t position;
  ecs_id_t sprite;
  ecs_id_t velocity;
  ecs_id_t weapon;
} Components;

/*
 * Systems
 */
typedef struct {
  ecs_id_t enemy_spawn;
  ecs_id_t input;
  ecs_id_t movement;
  ecs_id_t render;
  ecs_id_t weapon;
} Systems;

void register_components(void);
void register_systems(void);
void update_system_callbacks(void);
