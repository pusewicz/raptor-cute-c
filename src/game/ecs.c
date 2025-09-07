#include "ecs.h"

#include "../engine/game_state.h"
#include "factories.h"

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wsign-conversion"
  #pragma GCC diagnostic ignored "-Wfloat-conversion"
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
#include <stddef.h>

#if defined(__GNUC__) || defined(__clang__)
  #pragma GCC diagnostic pop
#endif

// System implementations
static ecs_ret_t s_update_input_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)entities;
  (void)entity_count;

  GameState *state = (GameState *)udata;
  input_t   *input = ecs_get(ecs, state->player_entity, state->components.input);

  input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
  input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
  input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
  input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
  input->shoot = cf_key_down(CF_KEY_SPACE) || cf_mouse_down(CF_MOUSE_BUTTON_LEFT);

  return 0;
}

static ecs_ret_t s_update_movement_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;

  GameState *state = (GameState *)udata;

  for (int i = 0; i < entity_count; i++) {
    ecs_id_t entity_id = entities[i];
    CF_V2   *pos       = ecs_get(ecs, entity_id, state->components.position);
    CF_V2   *vel       = ecs_get(ecs, entity_id, state->components.velocity);

    // TODO: Special case for player for now
    if (state->player_entity == entity_id) {
      input_t *input = ecs_get(ecs, state->player_entity, state->components.input);
      float    speed = 1.0f;
      vel->x         = 0.0f;
      vel->y         = 0.0f;
      if (input->up)
        vel->y += speed;
      if (input->down)
        vel->y -= speed;
      if (input->left)
        vel->x -= speed;
      if (input->right)
        vel->x += speed;
    }

    pos->x += vel->x;
    pos->y += vel->y;
  }

  return 0;
}

static ecs_ret_t s_update_weapon_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)entities;
  (void)entity_count;

  GameState *state  = (GameState *)udata;
  weapon_t  *weapon = ecs_get(ecs, state->player_entity, state->components.weapon);

  if (weapon->time_since_shot < weapon->cooldown) {
    weapon->time_since_shot += (float)dt;
    return 0;
  }

  input_t *input = ecs_get(ecs, state->player_entity, state->components.input);
  if (input->shoot) {
    weapon->time_since_shot = 0.0f;
    CF_V2 *pos              = ecs_get(ecs, state->player_entity, state->components.position);
    make_bullet(state, pos->x, pos->y, cf_v2(0, 1));
  }

  return 0;
}

static ecs_ret_t s_update_render_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  GameState *state = (GameState *)udata;

  cf_draw_push();
  cf_draw_scale(state->scale.x, state->scale.y);

  for (int i = 0; i < entity_count; i++) {
    CF_V2     *pos    = ecs_get(ecs, entities[i], state->components.position);
    CF_Sprite *sprite = ecs_get(ecs, entities[i], state->components.sprite);

    cf_sprite_update(sprite);
    cf_draw_push();
    cf_draw_translate_v2(*pos);
    cf_draw_sprite(sprite);
    cf_draw_pop();
  }

  cf_draw_pop();

  return 0;
}

void register_components(GameState *state) {
  if (!state->components_registered) {
    state->components.input      = ecs_register_component(state->ecs, sizeof(input_t), NULL, NULL);
    state->components.position   = ecs_register_component(state->ecs, sizeof(CF_V2), NULL, NULL);
    state->components.sprite     = ecs_register_component(state->ecs, sizeof(CF_Sprite), NULL, NULL);
    state->components.velocity   = ecs_register_component(state->ecs, sizeof(CF_V2), NULL, NULL);
    state->components.weapon     = ecs_register_component(state->ecs, sizeof(weapon_t), NULL, NULL);
    state->components_registered = true;
  }
}

void register_systems(GameState *state) {
  if (!state->systems_registered) {
    // First time registration
    state->systems.input    = ecs_register_system(state->ecs, s_update_input_system, NULL, NULL, state);
    state->systems.movement = ecs_register_system(state->ecs, s_update_movement_system, NULL, NULL, state);
    state->systems.weapon   = ecs_register_system(state->ecs, s_update_weapon_system, NULL, NULL, state);
    state->systems.render   = ecs_register_system(state->ecs, s_update_render_system, NULL, NULL, state);

    // Define input system required components
    ecs_require_component(state->ecs, state->systems.input, state->components.input);
    ecs_require_component(state->ecs, state->systems.input, state->components.position);

    // Define movement system required components
    ecs_require_component(state->ecs, state->systems.movement, state->components.position);
    ecs_require_component(state->ecs, state->systems.movement, state->components.velocity);

    // Define render system required components
    ecs_require_component(state->ecs, state->systems.render, state->components.position);
    ecs_require_component(state->ecs, state->systems.render, state->components.sprite);

    state->systems_registered = true;
  }
}

void update_system_callbacks(GameState *state) {
  ecs_t *ecs = state->ecs;

  // Update each system's callback to the current function address
  ecs->systems[state->systems.input].system_cb    = s_update_input_system;
  ecs->systems[state->systems.movement].system_cb = s_update_movement_system;
  ecs->systems[state->systems.render].system_cb   = s_update_render_system;
  ecs->systems[state->systems.weapon].system_cb   = s_update_weapon_system;
}

void update_systems(GameState *state) { ecs_update_systems(state->ecs, CF_DELTA_TIME); }
