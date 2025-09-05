#pragma once

#include <cute_alloc.h>
#include <cute_color.h>
#include <pico_ecs.h>

typedef struct Platform Platform;

typedef struct GameState {
  Platform *platform;
  CF_Color  bg_color;
  CF_Arena  permanent_arena;
  CF_Arena  stage_arena;
  CF_Arena  scratch_arena;
  ecs_t    *ecs;
  ecs_id_t  player_entity;
  
  // Component IDs - persist across hot reloads
  ecs_id_t PosComp;
  ecs_id_t VelComp;
  ecs_id_t InputComp;
  ecs_id_t SpriteComp;
  
  // System IDs - persist across hot reloads
  ecs_id_t InputSystem;
  ecs_id_t MovementSystem;
  ecs_id_t RenderSystem;
  
  // Track if components/systems have been registered
  bool components_registered;
  bool systems_registered;
} GameState;

extern GameState *state;
