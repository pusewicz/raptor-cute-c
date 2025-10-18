#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

constexpr int BACKGROUND_SCROLL_SPRITE_COUNT = 6 * 3;

typedef struct BackgroundScroll {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprites[BACKGROUND_SCROLL_SPRITE_COUNT];  // Background sprite
    float     y_offset;                                 // Vertical offset for scrolling
    float     max_y_offset;                             // Maximum offset before resetting
    ZIndex    z_index;                                  // Rendering order
} BackgroundScroll;

BackgroundScroll make_background_scroll(void);
void             update_background_scroll(void);
void             render_background_scroll(void);
