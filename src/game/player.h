#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

typedef struct Weapon {
    float cooldown;         // Time between shots in seconds
    float time_since_shot;  // Time since last shot in seconds
} Weapon;

typedef struct Input {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
} Input;

typedef struct Player {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    CF_Sprite booster_sprite;
    Input     input;
    Collider  collider;
    Weapon    weapon;
    bool      is_alive;
    bool      is_invincible;
    float     invincibility_timer;
    float     respawn_delay;
    ZIndex    z_index;  // Rendering order
} Player;

typedef struct PlayerBullet {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    Collider  collider;
    bool      is_alive;
    ZIndex    z_index;  // Rendering order
} PlayerBullet;

Player       make_player(float x, float y);
PlayerBullet make_player_bullet(float x, float y, CF_V2 direction);
void         spawn_player_bullet(PlayerBullet player_bullet);
void         cleanup_player_bullets(void);
void         damage_player(void);
void         update_player(Player* player);
void         render_player(Player* player);
