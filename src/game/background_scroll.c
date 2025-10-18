#include "background_scroll.h"

#include <cute_draw.h>
#include <cute_math.h>
#include <cute_sprite.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "asset/sprite.h"

BackgroundScroll make_background_scroll(void) {
    auto background_scroll = (BackgroundScroll){
        .position = cf_v2(0, 0),
        .velocity = cf_v2(0, 0.5f),
    };

    for (int i = 0; i < BACKGROUND_SCROLL_SPRITE_COUNT; ++i) {
        load_sprite(&background_scroll.sprites[i], "assets/background.ase");

        // Set the initial frame to 0 or 1 based on the index to create a
        // checkerboard pattern
        cf_sprite_set_frame(&background_scroll.sprites[i], i % 2);
    }

    background_scroll.max_y_offset = background_scroll.sprites[0].h;

    return background_scroll;
}

void update_background_scroll() {
    g_state->background_scroll.y_offset += 0.1f;
    if (g_state->background_scroll.y_offset >= g_state->background_scroll.max_y_offset) {
        g_state->background_scroll.y_offset = 0;
    }
}

void render_background_scroll(void) {
    cf_draw() {
        cf_draw_translate(
            0,
            g_state->canvas_size.y / 2.0f - g_state->background_scroll.y_offset +
                g_state->background_scroll.max_y_offset * 0.5f
        );
        int i = 0;
        for (int y = 0; y < (BACKGROUND_SCROLL_SPRITE_COUNT / 3); ++y) {
            for (int x = -1; x <= 1; ++x) {
                CF_Sprite* sprite = &g_state->background_scroll.sprites[i];
                cf_draw() {
                    cf_draw_translate(x * sprite->w, -y * sprite->h);
                    cf_sprite_update(sprite);
                    cf_sprite_draw(sprite);
                }
                ++i;
            }
        }
    }
}
