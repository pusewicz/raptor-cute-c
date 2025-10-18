#include "input.h"

#include <cute_input.h>

#include "../engine/game_state.h"

void update_input(Input* input) {
    // TODO: Remove this?
    if (!g_state->player.is_alive) {
        input->up    = false;
        input->down  = false;
        input->left  = false;
        input->right = false;
        input->shoot = false;

        return;
    }

    input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
    input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
    input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
    input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
    input->shoot = cf_key_down(CF_KEY_SPACE) || cf_mouse_down(CF_MOUSE_BUTTON_LEFT);
}
