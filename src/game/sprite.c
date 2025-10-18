#include "sprite.h"

#include <cute_draw.h>
#include <cute_math.h>
#include <cute_sprite.h>

#include "../engine/cute_macros.h"
#include "component.h"

void render_sprite(CF_Sprite* sprite, CF_V2 position, ZIndex z_index) {
    cf_sprite_update(sprite);
    cf_draw() {
        cf_draw_layer(z_index) {
            cf_draw_translate_v2(position);
            cf_draw_sprite(sprite);
        }
    }
}
