#include "state.h"

#include "../engine/game_state.h"

#include <SDL3/SDL_log.h>
#include <stdbool.h>
#include <stddef.h>

bool validate_game_state(void) {
  if (g_state == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "g_state is NULL");
    return false;
  }
  if (g_state->platform == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "g_state->platform is NULL");
    return false;
  }
  if (g_state->canvas_size.x <= 0 || g_state->canvas_size.y <= 0) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM,
                 "g_state->canvas_size is invalid: (%.2f, %.2f)",
                 g_state->canvas_size.x,
                 g_state->canvas_size.y);
    return false;
  }
  if (g_state->scale.x <= 0 || g_state->scale.y <= 0) {
    SDL_LogError(
        SDL_LOG_CATEGORY_CUSTOM, "g_state->scale is invalid: (%.2f, %.2f)", g_state->scale.x, g_state->scale.y);
    return false;
  }
  if (g_state->ecs == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "g_state->ecs is NULL");
    return false;
  }
  if (!g_state->systems_registered) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "g_state->systems_registered is false");
    return false;
  }
  return true;
}
