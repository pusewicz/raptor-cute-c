#include "ecs.h"

#include "../engine/game_state.h"

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wsign-conversion"
  #pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif

#define PICO_ECS_IMPLEMENTATION
#include <SDL3/SDL_log.h>
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <pico_ecs.h>

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif

/*
 * Component IDs
 */
ecs_id_t PosComp;       // Position component
ecs_id_t VelComp;       // Velocity component
ecs_id_t InputComp;     // Input component
ecs_id_t SpriteComp;    // Sprite component

/*
 * Systems
 */
ecs_id_t InputSystem;
ecs_id_t MovementSystem;
ecs_id_t RenderSystem;

static ecs_ret_t s_update_input_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)entities;
  (void)entity_count;

  SDL_Log("Updating input for player entity\n");

  GameState *state = (GameState *)udata;
  input_t   *input = ecs_get(ecs, state->player_entity, InputComp);

  input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
  input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
  input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
  input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);

  return 0;
}

static ecs_ret_t s_update_movement_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)udata;

  SDL_Log("Updating movement for %d entities\n", entity_count);

  for (int i = 0; i < entity_count; i++) {
    (void)dt;

    ecs_id_t entity_id = entities[i];
    CF_V2   *pos       = ecs_get(ecs, entity_id, PosComp);
    CF_V2   *vel       = ecs_get(ecs, entity_id, VelComp);
    input_t *input     = ecs_get(ecs, entity_id, InputComp);

    float speed = 3.0f;
    vel->x      = 0.0f;
    vel->y      = 0.0f;
    if (input->up)
      vel->y += speed;
    if (input->down)
      vel->y -= speed;
    if (input->left)
      vel->x -= speed;
    if (input->right)
      vel->x += speed;
    pos->x += vel->x;
    pos->y += vel->y;
  }

  return 0;
}

static ecs_ret_t s_update_render_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)udata;

  SDL_Log("Rendering %d entities\n", entity_count);

  for (int i = 0; i < entity_count; i++) {
    CF_V2     *pos    = ecs_get(ecs, entities[i], PosComp);
    CF_Sprite *sprite = ecs_get(ecs, entities[i], SpriteComp);

    cf_sprite_update(sprite);
    cf_draw_push();
    cf_draw_translate_v2(*pos);
    cf_draw_sprite(sprite);
    cf_draw_pop();
  }

  return 0;
}

void register_components(GameState *state) {
  PosComp    = ecs_register_component(state->ecs, sizeof(CF_V2), nullptr, nullptr);
  VelComp    = ecs_register_component(state->ecs, sizeof(CF_V2), nullptr, nullptr);
  InputComp  = ecs_register_component(state->ecs, sizeof(input_t), nullptr, nullptr);
  SpriteComp = ecs_register_component(state->ecs, sizeof(CF_Sprite), nullptr, nullptr);
}

void register_systems(GameState *state) {
  // Register systems
  InputSystem    = ecs_register_system(state->ecs, s_update_input_system, nullptr, nullptr, state);
  MovementSystem = ecs_register_system(state->ecs, s_update_movement_system, nullptr, nullptr, state);
  RenderSystem   = ecs_register_system(state->ecs, s_update_render_system, nullptr, nullptr, state);

  // Define input system required components
  ecs_require_component(state->ecs, InputSystem, InputComp);
  ecs_require_component(state->ecs, InputSystem, PosComp);

  // Define movement system required components
  ecs_require_component(state->ecs, MovementSystem, PosComp);
  ecs_require_component(state->ecs, MovementSystem, VelComp);
  ecs_require_component(state->ecs, MovementSystem, InputComp);

  // Define render system required components
  ecs_require_component(state->ecs, RenderSystem, PosComp);
  ecs_require_component(state->ecs, RenderSystem, SpriteComp);
}

void update_systems(GameState *state) { ecs_update_systems(state->ecs, CF_DELTA_TIME); }
