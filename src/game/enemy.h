#pragma once

#include <cute_math.h>
#include <cute_sprite.h>

#include "component.h"

typedef enum EnemyType {
    ENEMY_TYPE_ALAN,
    ENEMY_TYPE_BON_BON,
    ENEMY_TYPE_LIPS,
    ENEMY_TYPE_COUNT,
} EnemyType;

typedef struct Health {
    int current;
    int maximum;
} Health;

typedef struct Enemy {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    Collider  collider;
    bool      is_alive;
    ZIndex    z_index;          // Rendering order
    Health    health;
    int       score;
    float     cooldown;         // Time between shots in seconds
    float     time_since_shot;  // Time since last shot in seconds
    float     shoot_chance;     // Probability of shooting when cooldown ready (0.0-1.0)
    EnemyType type;
} Enemy;

typedef struct EnemyBullet {
    CF_V2     position;
    CF_V2     velocity;
    CF_Sprite sprite;
    Collider  collider;
    bool      is_alive;
    ZIndex    z_index;  // Rendering order
} EnemyBullet;

Enemy       make_enemy_of_type(float x, float y, EnemyType type);
Enemy       make_random_enemy(float x, float y);
EnemyBullet make_enemy_bullet(float x, float y, CF_V2 direction);
void        spawn_enemy_bullet(EnemyBullet bullet);
void        spawn_enemy(Enemy enemy);
void        update_enemy(Enemy* enemy);
void        cleanup_enemies(void);
void        cleanup_enemy_bullets(void);
