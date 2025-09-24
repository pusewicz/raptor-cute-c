#include "game_state.h"

#include "log.h"

#include <stdbool.h>
#include <stddef.h>

bool validate_game_state(void) {
  if (g_state == NULL) {
    APP_WARN("g_state is NULL");
    return false;
  }
  if (g_state->platform == NULL) {
    APP_WARN("g_state->platform is NULL");
    return false;
  }
  if (g_state->canvas_size.x <= 0 || g_state->canvas_size.y <= 0) {
    APP_WARN("g_state->canvas_size is invalid: (%.2f, %.2f)", g_state->canvas_size.x, g_state->canvas_size.y);
    return false;
  }
  if (g_state->scale <= 0) {
    APP_WARN("g_state->scale is invalid: (%.2f)", g_state->scale);
    return false;
  }
  if (g_state->ecs == NULL) {
    APP_WARN("g_state->ecs is NULL");
    return false;
  }
  return true;
}
