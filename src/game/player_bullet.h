#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

typedef struct PlayerBullet {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    Collider  collider;
    bool      is_alive;
    ZIndex    z_index;  // Rendering order
} PlayerBullet;

PlayerBullet make_player_bullet(float x, float y, CF_V2 direction);
void         spawn_player_bullet(PlayerBullet player_bullet);
void         cleanup_player_bullets(void);
