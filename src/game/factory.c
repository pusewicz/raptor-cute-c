#include "factory.h"

#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>
#include <pico_ecs.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "asset/sprite.h"
#include "ecs.h"

/*
 * Player
 */

constexpr float WEAPON_DEFAULT_COOLDOWN = 0.15f;

ecs_id_t make_player(float x, float y) {
    auto id      = make_entity();

    // Add position
    auto pos     = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x       = x;
    pos->y       = y;

    // Add velocity
    auto vel     = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x       = 0.0f;
    vel->y       = 0.0f;

    // Add input controls
    auto input   = ECS_ADD_COMPONENT(id, InputComponent);
    input->up    = false;
    input->down  = false;
    input->left  = false;
    input->right = false;

    // Add sprite
    auto sprite  = ECS_ADD_COMPONENT(id, PlayerSpriteComponent);
    load_sprite(&sprite->sprite, "assets/player.ase");
    load_sprite(&sprite->booster_sprite, "assets/boosters.ase");
    sprite->booster_sprite.offset.y = -sprite->sprite.h;
    sprite->z_index                 = Z_PLAYER_SPRITE;
    cf_sprite_play(&sprite->sprite, "default");
    cf_sprite_play(&sprite->booster_sprite, "default");

    // Add collider
    auto collider              = ECS_ADD_COMPONENT(id, ColliderComponent);
    collider->half_extents     = cf_v2(sprite->sprite.w / 4.0, sprite->sprite.h / 4.0);

    // Add weapon
    auto weapon                = ECS_ADD_COMPONENT(id, WeaponComponent);
    weapon->cooldown           = WEAPON_DEFAULT_COOLDOWN;
    weapon->time_since_shot    = 0.0f;

    // Add tag
    auto tag                   = ECS_ADD_COMPONENT(id, TagComponent);
    *tag                       = TAG_PLAYER;

    // Add player state
    auto state                 = ECS_ADD_COMPONENT(id, PlayerStateComponent);
    state->is_alive            = true;
    state->is_invincible       = false;
    state->invincibility_timer = 0.0f;
    state->respawn_delay       = 0.0f;

    return id;
}

/*
 * Background Scroll
 */

ecs_id_t make_background_scroll(void) {
    auto id = make_entity();

    // Add background scroll component
    ECS_ADD_COMPONENT(id, BackgroundScrollComponent);

    // Add position
    auto pos = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x   = 0.0f;
    pos->y   = 0.0f;

    // Add velocity
    auto vel = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->y   = 0.5f;  // Scroll down at a constant speed

    return id;
}

/*
 * Bullets
 */

constexpr float BULLET_DEFAULT_SPEED = 3.0f;

ecs_id_t make_bullet(float x, float y, CF_V2 direction) {
    auto id     = make_entity();

    // Add position
    auto pos    = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x      = x;
    pos->y      = y;

    // Add velocity
    auto vel    = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x      = 0.0f;
    vel->y      = BULLET_DEFAULT_SPEED * direction.y;

    // Add sprite
    auto sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    load_sprite(&sprite->sprite, "assets/bullet.png");
    sprite->z_index        = Z_SPRITES;

    // Add collider
    auto collider          = ECS_ADD_COMPONENT(id, ColliderComponent);
    collider->half_extents = cf_v2(sprite->sprite.w / 4.2, sprite->sprite.h / 4.2);

    // Add tag
    auto tag               = ECS_ADD_COMPONENT(id, TagComponent);
    *tag                   = TAG_BULLET;

    return id;
}

/*
 * Enemies
 */

constexpr float ENEMY_DEFAULT_SPEED = 0.5f;

ecs_id_t make_enemy_of_type(float x, float y, EnemyType type) {
    auto id            = make_entity();

    // Add position
    auto pos           = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x             = x;
    pos->y             = y;

    // Add velocity
    auto vel           = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x             = 0.0f;
    vel->y             = -ENEMY_DEFAULT_SPEED;

    // Add sprite based on type
    auto        sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    const char* sprite_path;
    int         score_value;

    switch (type) {
        case ENEMY_TYPE_ALAN:
            sprite_path = "assets/alan.ase";
            score_value = 100;
            break;
        case ENEMY_TYPE_BON_BON:
            sprite_path = "assets/bon_bon.ase";
            score_value = 150;
            break;
        case ENEMY_TYPE_LIPS:
            sprite_path = "assets/lips.ase";
            score_value = 200;
            break;
        default:
            sprite_path = "assets/alan.ase";
            score_value = 100;
            break;
    }

    load_sprite(&sprite->sprite, sprite_path);
    sprite->z_index        = Z_SPRITES;

    // Add collider
    auto collider          = ECS_ADD_COMPONENT(id, ColliderComponent);
    collider->half_extents = cf_v2(sprite->sprite.w / 3.0f, sprite->sprite.h / 3.0f);

    // Add score
    auto score             = ECS_ADD_COMPONENT(id, ScoreComponent);
    *score                 = score_value;

    // Add tag
    auto tag               = ECS_ADD_COMPONENT(id, TagComponent);
    *tag                   = TAG_ENEMY;

    return id;
}

// Convenience function that creates a random enemy type
ecs_id_t make_enemy(float x, float y) {
    EnemyType types[] = {ENEMY_TYPE_ALAN, ENEMY_TYPE_BON_BON, ENEMY_TYPE_LIPS};
    int       type    = cf_rnd_range_int(&g_state->rnd, 0, 2);
    return make_enemy_of_type(x, y, types[type]);
}

/*
 * Explosion
 */

ecs_id_t make_explosion(float x, float y) {
    auto id     = make_entity();

    // Add position
    auto pos    = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x      = x;
    pos->y      = y;

    // Add sprite
    auto sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    load_sprite(&sprite->sprite, "assets/explosion.ase");
    sprite->z_index = Z_SPRITES;
    cf_sprite_set_loop(&sprite->sprite, false);

    return id;
}
