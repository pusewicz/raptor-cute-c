#include "factories.h"

#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>
#include <pico_ecs.h>
#include <stddef.h>

#include "../engine/game_state.h"
#include "../engine/log.h"
#include "ecs.h"

/*
 * Player
 */

constexpr float WEAPON_DEFAULT_COOLDOWN = 0.15f;

ecs_id_t make_player(float x, float y) {
    ecs_id_t id = make_entity();

    // Add position
    PositionComponent* pos = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x                 = x;
    pos->y                 = y;

    // Add velocity
    VelocityComponent* vel = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x                 = 0.0f;
    vel->y                 = 0.0f;

    // Add input controls
    InputComponent* input = ECS_ADD_COMPONENT(id, InputComponent);
    input->up             = false;
    input->down           = false;
    input->left           = false;
    input->right          = false;

    // Add sprite
    SpriteComponent* sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    CF_Result        result;
    *sprite = cf_make_easy_sprite_from_png("assets/player.png", &result);
    if (cf_is_error(result)) {
        APP_ERROR("Failed to load player sprite: %s\n", result.details);
    }

    // Add weapon
    WeaponComponent* weapon = ECS_ADD_COMPONENT(id, WeaponComponent);
    weapon->cooldown        = WEAPON_DEFAULT_COOLDOWN;
    weapon->time_since_shot = 0.0f;

    // Add tag
    TagComponent* tag = ECS_ADD_COMPONENT(id, TagComponent);
    *tag              = TAG_PLAYER;

    return id;
}

/*
 * Background Scroll
 */

ecs_id_t make_background_scroll(void) {
    ecs_id_t id = make_entity();

    // Add background scroll component
    ECS_ADD_COMPONENT(id, BackgroundScrollComponent);

    // Add position
    PositionComponent* pos = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x                 = 0.0f;
    pos->y                 = 0.0f;

    // Add velocity
    VelocityComponent* vel = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->y                 = 0.5f;  // Scroll down at a constant speed

    return id;
}

/*
 * Bullets
 */

constexpr float BULLET_DEFAULT_SPEED = 3.0f;

ecs_id_t make_bullet(float x, float y, CF_V2 direction) {
    ecs_id_t id = make_entity();

    // Add position
    PositionComponent* pos = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x                 = x;
    pos->y                 = y;

    // Add velocity
    VelocityComponent* vel = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x                 = 0.0f;
    vel->y                 = BULLET_DEFAULT_SPEED * direction.y;

    // Add sprite
    SpriteComponent* sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    CF_Result        result;
    *sprite = cf_make_easy_sprite_from_png("assets/bullet.png", &result);
    if (cf_is_error(result)) {
        APP_ERROR("Failed to load bullet sprite: %s\n", result.details);
    }

    // Add collider
    ColliderComponent* collider = ECS_ADD_COMPONENT(id, ColliderComponent);
    collider->half_extents      = cf_v2(sprite->w / 4.2, sprite->h / 4.2);

    // Add tag
    TagComponent* tag = ECS_ADD_COMPONENT(id, TagComponent);
    *tag              = TAG_BULLET;

    return id;
}

/*
 * Enemies
 */

constexpr float ENEMY_DEFAULT_SPEED = 0.5f;

ecs_id_t make_enemy(float x, float y) {
    ecs_id_t id = make_entity();

    // Add position
    PositionComponent* pos = ECS_ADD_COMPONENT(id, PositionComponent);
    pos->x                 = x;
    pos->y                 = y;

    // Add velocity
    VelocityComponent* vel = ECS_ADD_COMPONENT(id, VelocityComponent);
    vel->x                 = 0.0f;
    vel->y                 = -ENEMY_DEFAULT_SPEED;

    // Add sprite
    SpriteComponent* sprite = ECS_ADD_COMPONENT(id, SpriteComponent);
    *sprite                 = cf_make_sprite("assets/alan.ase");

    // Add collider
    ColliderComponent* collider = ECS_ADD_COMPONENT(id, ColliderComponent);
    collider->half_extents      = cf_v2(sprite->w / 2.0f, sprite->h / 2.0f);

    // Add tag
    TagComponent* tag = ECS_ADD_COMPONENT(id, TagComponent);
    *tag              = TAG_ENEMY;

    return id;
}

/*
 * Enemy Spawner
 */

ecs_id_t make_enemy_spawner(void) {
    ecs_id_t id = make_entity();

    EnemySpawnComponent* spawn = ECS_ADD_COMPONENT(id, EnemySpawnComponent);
    // Spawn an enemy every 2 seconds
    spawn->spawn_interval = spawn->time_since_last_spawn = 2.0f;
    spawn->max_enemies                                   = 5;
    spawn->current_enemy_count                           = 0;

    return id;
}
