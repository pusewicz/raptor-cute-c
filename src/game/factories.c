#include "factories.h"

#include "../engine/game_state.h"
#include "ecs.h"

#include <SDL3/SDL_log.h>
#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>

ecs_id_t make_player(GameState *state, float x, float y) {
  ecs_id_t id = ecs_create(state->ecs);

  CF_V2 *pos = ecs_add(state->ecs, id, state->PosComp, nullptr);
  pos->x     = x;
  pos->y     = y;

  CF_V2 *vel = ecs_add(state->ecs, id, state->VelComp, nullptr);
  vel->x     = 0.0f;
  vel->y     = 0.0f;

  input_t *input = ecs_add(state->ecs, id, state->InputComp, nullptr);
  input->up      = false;
  input->down    = false;
  input->left    = false;
  input->right   = false;

  CF_Sprite *sprite = ecs_add(state->ecs, id, state->SpriteComp, nullptr);
  CF_Result  result;
  *sprite = cf_make_easy_sprite_from_png("assets/player.png", &result);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load player sprite: %s\n", result.details);
  }

  return id;
}
