#pragma once

#include <cute_math.h>

typedef struct Collider {
    CF_V2 half_extents;
} Collider;

typedef enum ZIndex {
    Z_BACKGROUND = 0,
    Z_SPRITES,
    Z_PLAYER_SPRITE,
    Z_PARTICLES,
    Z_UI,
    Z_MAX,
} ZIndex;
