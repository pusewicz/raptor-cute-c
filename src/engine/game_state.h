#pragma once

#include <cute_alloc.h>
#include <cute_color.h>
#include <pico_ecs.h>

typedef struct Platform Platform;

typedef struct {
  ecs_id_t position;
  ecs_id_t velocity;
  ecs_id_t input;
  ecs_id_t sprite;
} Components;

typedef struct {
  ecs_id_t input;
  ecs_id_t movement;
  ecs_id_t render;
} Systems;

typedef struct GameState {
  Platform *platform;
  CF_Color  bg_color;
  CF_Arena  permanent_arena;
  CF_Arena  stage_arena;
  CF_Arena  scratch_arena;
  ecs_t    *ecs;
  ecs_id_t  player_entity;
  
  // Component and System IDs - persist across hot reloads
  Components components;
  Systems    systems;
  
  // Track if components/systems have been registered
  bool components_registered;
  bool systems_registered;
} GameState;

extern GameState *state;
