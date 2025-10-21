#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

typedef struct StarParticle {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    float     size;
    int       layer;            // Parallax layer index
    float     parallax_offset;  // Horizontal parallax offset for rendering
    ZIndex    z_index;
} StarParticle;

void init_star_particles(void);
void update_star_particles(void);
