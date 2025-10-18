#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

typedef struct Explosion {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    Collider  collider;
    bool      is_alive;
    ZIndex    z_index;  // Rendering order
} Explosion;

Explosion make_explosion(float x, float y);
void      spawn_explosion(Explosion explosion);
void      cleanup_explosions(void);
