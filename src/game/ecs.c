#include "ecs.h"

#include "../engine/game_state.h"

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wsign-conversion"
  #pragma GCC diagnostic ignored "-Wshorten-64-to-32"
#endif

#define PICO_ECS_IMPLEMENTATION
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_math.h>
#include <cute_sprite.h>
#include <cute_time.h>
#include <pico_ecs.h>

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif

// System implementations
static ecs_ret_t s_update_input_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)entities;
  (void)entity_count;

  GameState *state = (GameState *)udata;
  input_t   *input = ecs_get(ecs, state->player_entity, state->InputComp);

  input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
  input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
  input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
  input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);

  return 0;
}

static ecs_ret_t s_update_movement_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  GameState *state = (GameState *)udata;

  for (int i = 0; i < entity_count; i++) {
    (void)dt;

    ecs_id_t entity_id = entities[i];
    CF_V2   *pos       = ecs_get(ecs, entity_id, state->PosComp);
    CF_V2   *vel       = ecs_get(ecs, entity_id, state->VelComp);
    input_t *input     = ecs_get(ecs, entity_id, state->InputComp);

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
  GameState *state = (GameState *)udata;

  for (int i = 0; i < entity_count; i++) {
    CF_V2     *pos    = ecs_get(ecs, entities[i], state->PosComp);
    CF_Sprite *sprite = ecs_get(ecs, entities[i], state->SpriteComp);

    cf_sprite_update(sprite);
    cf_draw_push();
    cf_draw_translate_v2(*pos);
    cf_draw_sprite(sprite);
    cf_draw_pop();
  }

  return 0;
}

void register_components(GameState *state) {
  if (!state->components_registered) {
    state->PosComp               = ecs_register_component(state->ecs, sizeof(CF_V2), nullptr, nullptr);
    state->VelComp               = ecs_register_component(state->ecs, sizeof(CF_V2), nullptr, nullptr);
    state->InputComp             = ecs_register_component(state->ecs, sizeof(input_t), nullptr, nullptr);
    state->SpriteComp            = ecs_register_component(state->ecs, sizeof(CF_Sprite), nullptr, nullptr);
    state->components_registered = true;
  }
}

void register_systems(GameState *state) {
  if (!state->systems_registered) {
    // First time registration
    state->InputSystem    = ecs_register_system(state->ecs, s_update_input_system, nullptr, nullptr, state);
    state->MovementSystem = ecs_register_system(state->ecs, s_update_movement_system, nullptr, nullptr, state);
    state->RenderSystem   = ecs_register_system(state->ecs, s_update_render_system, nullptr, nullptr, state);

    // Define input system required components
    ecs_require_component(state->ecs, state->InputSystem, state->InputComp);
    ecs_require_component(state->ecs, state->InputSystem, state->PosComp);

    // Define movement system required components
    ecs_require_component(state->ecs, state->MovementSystem, state->PosComp);
    ecs_require_component(state->ecs, state->MovementSystem, state->VelComp);
    ecs_require_component(state->ecs, state->MovementSystem, state->InputComp);

    // Define render system required components
    ecs_require_component(state->ecs, state->RenderSystem, state->PosComp);
    ecs_require_component(state->ecs, state->RenderSystem, state->SpriteComp);

    state->systems_registered = true;
  }
}

void update_system_callbacks(GameState *state) {
  ecs_t *ecs = state->ecs;

  // Update each system's callback to the current function address
  ecs->systems[state->InputSystem].system_cb    = s_update_input_system;
  ecs->systems[state->MovementSystem].system_cb = s_update_movement_system;
  ecs->systems[state->RenderSystem].system_cb   = s_update_render_system;
}

void update_systems(GameState *state) { ecs_update_systems(state->ecs, CF_DELTA_TIME); }
