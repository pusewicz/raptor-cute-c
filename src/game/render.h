#pragma once

#include <cute_math.h>

// Macro for rendering simple entity arrays with sprite rendering
#define RENDER_ENTITY_ARRAY(array, count, sprite_field, position_field, z_index_field)                    \
    do {                                                                                                  \
        for (size_t i = 0; i < (count); ++i) {                                                            \
            render_sprite(&(array)[i].sprite_field, (array)[i].position_field, (array)[i].z_index_field); \
        }                                                                                                 \
    } while (0)

// Macro for rendering debug bounding boxes for entity arrays
#define RENDER_DEBUG_BBOXES(array, count, position_field, collider_field)                                      \
    do {                                                                                                       \
        cf_draw_push();                                                                                        \
        cf_draw_push_color(cf_color_blue());                                                                   \
        for (size_t i = 0; i < (count); ++i) {                                                                 \
            auto entity = &(array)[i];                                                                         \
            auto aabb_collider =                                                                               \
                cf_make_aabb_center_half_extents(entity->position_field, entity->collider_field.half_extents); \
            cf_draw_quad(aabb_collider, 0, 0);                                                                 \
        }                                                                                                      \
        cf_draw_pop_color();                                                                                   \
        cf_draw_pop();                                                                                         \
    } while (0)
