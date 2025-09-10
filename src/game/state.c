#include "state.h"

#include "../engine/game_state.h"

#include <SDL3/SDL_log.h>
#include <stdbool.h>
#include <stddef.h>

bool validate_game_state(GameState *state) {
  if (state == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state is NULL");
    return false;
  }
  if (state->platform == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state->platform is NULL");
    return false;
  }
  if (state->canvas_size.x <= 0 || state->canvas_size.y <= 0) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
                 "state->canvas_size is invalid: (%.2f, %.2f)",
                 state->canvas_size.x,
                 state->canvas_size.y);
    return false;
  }
  if (state->scale.x <= 0 || state->scale.y <= 0) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state->scale is invalid: (%.2f, %.2f)", state->scale.x, state->scale.y);
    return false;
  }
  if (state->ecs == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state->ecs is NULL");
    return false;
  }
  if (!state->components_registered) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state->components_registered is false");
    return false;
  }
  if (!state->systems_registered) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "state->systems_registered is false");
    return false;
  }
  return true;
}
