#include "game_state.h"

#include "log.h"

bool validate_game_state(void) {
    if (g_state == nullptr) {
        APP_WARN("g_state is null");
        return false;
    }
    if (g_state->platform == nullptr) {
        APP_WARN("g_state->platform is null");
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
    if (g_state->ecs == nullptr) {
        APP_WARN("g_state->ecs is null");
        return false;
    }
    return true;
}
