#include "intro.h"

#include <cute_app.h>
#include <cute_draw.h>

#include "../../engine/cute_macros.h"
#include "../../engine/game_state.h"
#include "../input.h"
#include "../render.h"
#include "../scene.h"

void intro_init(void) {
    // Initialize intro scene resources here
}

bool intro_update(void) {
    update_input(&g_state->player.input);

    // Transition to gameplay when shoot button is pressed
    if (g_state->player.input.shoot) {
        scene_switch(SCENE_GAMEPLAY);
        return true;
    }

    return true;
}

void intro_render(void) {
    cf_draw() {
        cf_draw_layer(Z_UI) {
            cf_font("TinyAndChunky") {
                cf_push_font_size(12);

                const char* text         = "INTRO";
                float       text_width   = cf_text_width(text, -1);
                float       text_height  = cf_text_height(text, -1);
                float       center_x     = -text_width / 2;
                float       center_y     = text_height / 2;

                // Draw text with shadow
                cf_draw_color(cf_make_color_rgb(20, 91, 132)) {
                    cf_draw_text(text, cf_v2(center_x + 1, center_y - 1), -1);
                }
                cf_draw_color(cf_color_white()) {
                    cf_draw_text(text, cf_v2(center_x, center_y), -1);
                }
            }
        }
    }
}

void intro_cleanup(void) {
    // Cleanup intro scene resources here
}
