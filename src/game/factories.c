#include "factories.h"

#include "../engine/game_state.h"
#include "ecs.h"

#include <SDL3/SDL_log.h>
#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>
#include <stdbool.h>
#include <stddef.h>

ecs_id_t make_player(GameState *state, float x, float y) {
  ecs_id_t id = ecs_create(state->ecs);

  // Add position
  CF_V2 *pos = ecs_add(state->ecs, id, state->components.position, NULL);
  pos->x     = x;
  pos->y     = y;

  // Add velocity
  CF_V2 *vel = ecs_add(state->ecs, id, state->components.velocity, NULL);
  vel->x     = 0.0f;
  vel->y     = 0.0f;

  // Add input controls
  input_t *input = ecs_add(state->ecs, id, state->components.input, NULL);
  input->up      = false;
  input->down    = false;
  input->left    = false;
  input->right   = false;

  // Add sprite
  CF_Sprite *sprite = ecs_add(state->ecs, id, state->components.sprite, NULL);
  CF_Result  result;
  *sprite = cf_make_easy_sprite_from_png("assets/player.png", &result);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load player sprite: %s\n", result.details);
  }

  // Add weapon
  weapon_t *weapon        = ecs_add(state->ecs, id, state->components.weapon, NULL);
  weapon->cooldown        = 0.5f;    // Half a second between shots
  weapon->time_since_shot = 0.0f;

  return id;
}

ecs_id_t make_bullet(GameState *state, float x, float y, CF_V2 direction) {
  SDL_Log("Making bullet at (%.2f, %.2f) going %s\n", x, y, 1 == direction.y ? "UP" : "DOWN");
  ecs_id_t id = ecs_create(state->ecs);

  bullet_t *bullet  = ecs_add(state->ecs, id, state->components.bullet, NULL);
  bullet->direction = direction;

  CF_V2 *pos = ecs_add(state->ecs, id, state->components.position, NULL);
  pos->x     = x;
  pos->y     = y;

  CF_V2 *vel = ecs_add(state->ecs, id, state->components.velocity, NULL);
  vel->x     = 0.0f;
  vel->y     = 1 == direction.y ? 2.0f : -2.0f;

  CF_Sprite *sprite = ecs_add(state->ecs, id, state->components.sprite, NULL);
  CF_Result  result;
  *sprite = cf_make_easy_sprite_from_png("assets/bullet.png", &result);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load bullet sprite: %s\n", result.details);
  }

  return id;
}
