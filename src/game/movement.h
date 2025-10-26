#pragma once

#include <cute_math.h>

static inline void update_movement(CF_V2* position, const CF_V2* velocity) {
    position->x += velocity->x;
    position->y += velocity->y;
}
