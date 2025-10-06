#pragma once

#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "pico_ecs.h"

/*
 * Convenience macros for ECS operations
 */

// Get a component of type T from an entity
#define ECS_GET(entity, T)        (T*)ecs_get(g_state->ecs, entity, g_state->components.COMPONENT_TO_UNDERSCORE(T))
#define ECS_READY(entity)         (ecs_is_ready(g_state->ecs, entity))
#define ECS_DESTROY(entity)       ecs_destroy(g_state->ecs, entity)
#define ECS_QUEUE_DESTROY(entity) ecs_queue_destroy(g_state->ecs, entity)

#define ECS_REGISTER_COMPONENT(T, ...)                                                                            \
    g_state->components.COMPONENT_TO_UNDERSCORE(T) = ecs_register_component(g_state->ecs, sizeof(T), __VA_ARGS__)
#define ECS_REGISTER_SYSTEM(name, ...)                                                    \
    g_state->systems.name = ecs_register_system(g_state->ecs, name##_system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_EACH(system, T)                                                                     \
    ecs_require_component(g_state->ecs, g_state->systems.system, g_state->components.COMPONENT_TO_UNDERSCORE(T));
#define ECS_REQUIRE_COMPONENT_1(system, T) ECS_REQUIRE_COMPONENT_EACH(system, T)
#define ECS_REQUIRE_COMPONENT_2(system, T, ...)  \
    ECS_REQUIRE_COMPONENT_EACH(system, T)        \
    ECS_REQUIRE_COMPONENT_1(system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_3(system, T, ...)  \
    ECS_REQUIRE_COMPONENT_EACH(system, T)        \
    ECS_REQUIRE_COMPONENT_2(system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_4(system, T, ...)  \
    ECS_REQUIRE_COMPONENT_EACH(system, T)        \
    ECS_REQUIRE_COMPONENT_3(system, __VA_ARGS__)

#define ECS_REQUIRE_COMPONENT_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define ECS_REQUIRE_COMPONENT(system, ...)                                                                                                            \
    ECS_REQUIRE_COMPONENT_GET_MACRO(__VA_ARGS__, ECS_REQUIRE_COMPONENT_4, ECS_REQUIRE_COMPONENT_3, ECS_REQUIRE_COMPONENT_2, ECS_REQUIRE_COMPONENT_1)( \
        system, __VA_ARGS__                                                                                                                           \
    )
#define ECS_UPDATE_SYSTEM(name) ecs_update_system(g_state->ecs, g_state->systems.name, CF_DELTA_TIME)
#define ECS_SET_SYSTEM_CALLBACKS(name)                                                             \
    ecs_set_system_callbacks(g_state->ecs, g_state->systems.name, name##_system, nullptr, nullptr)

// Add a component to an entity
#define ECS_ADD_COMPONENT(entity, T)                                                          \
    ((T*)add_component_impl(entity, g_state->components.COMPONENT_TO_UNDERSCORE(T), nullptr))

// Helper macros to convert CamelCase to snake_case for component names
#define COMPONENT_TO_UNDERSCORE(T)           UNDERSCORE_##T
#define UNDERSCORE_BackgroundScrollComponent background_scroll
#define UNDERSCORE_BulletComponent           bullet
#define UNDERSCORE_ColliderComponent         collider
#define UNDERSCORE_EnemyWeaponComponent      enemy_weapon
#define UNDERSCORE_InputComponent            input
#define UNDERSCORE_PlayerSpriteComponent     player_sprite
#define UNDERSCORE_PlayerStateComponent      player_state
#define UNDERSCORE_PositionComponent         position
#define UNDERSCORE_ScoreComponent            score
#define UNDERSCORE_SpriteComponent           sprite
#define UNDERSCORE_TagComponent              tag
#define UNDERSCORE_VelocityComponent         velocity
#define UNDERSCORE_WeaponComponent           weapon

// Convenience macro to create a new entity
#define make_entity() ecs_create(g_state->ecs)

/*
 * Background Scroll Component
 */

constexpr int BACKGROUND_SCROLL_SPRITE_COUNT = 6 * 3;

typedef struct BackgroundScrollComponent {
    CF_Sprite sprites[BACKGROUND_SCROLL_SPRITE_COUNT];  // Background sprite
    float     y_offset;                                 // Vertical offset for scrolling
    float     max_y_offset;                             // Maximum offset before resetting
} BackgroundScrollComponent;

/*
 * Bullet Component
 */

typedef struct BulletComponent {
    CF_V2 direction;
} BulletComponent;

/*
 * Collider Component
 */

typedef struct ColliderComponent {
    CF_V2 half_extents;
} ColliderComponent;

/*
 * Input Component
 */

typedef struct InputComponent {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
} InputComponent;

/*
 * Position Component
 */

typedef CF_V2 PositionComponent;

/*
 * Sprite Component
 */

typedef enum ZIndex {
    Z_BACKGROUND = 0,
    Z_SPRITES,
    Z_PLAYER_SPRITE,
    Z_UI,
} ZIndex;

typedef struct SpriteComponent {
    CF_Sprite sprite;
    ZIndex    z_index;  // Rendering order
} SpriteComponent;

typedef struct PlayerSpriteComponent {
    CF_Sprite sprite;
    CF_Sprite booster_sprite;
    ZIndex    z_index;  // Rendering order
} PlayerSpriteComponent;

/**
 * Score component
 */

typedef int ScoreComponent;

/*
 * Tag Component
 */

typedef enum TagComponent { TAG_PLAYER, TAG_ENEMY, TAG_PLAYER_BULLET, TAG_ENEMY_BULLET } TagComponent;

/*
 * Velocity Component
 */

typedef CF_V2 VelocityComponent;

/*
 * Weapon Component
 */

typedef struct WeaponComponent {
    float cooldown;         // Time between shots in seconds
    float time_since_shot;  // Time since last shot in seconds
} WeaponComponent;

/*
 * Enemy Weapon Component
 */

typedef struct EnemyWeaponComponent {
    float cooldown;         // Time between shots in seconds
    float time_since_shot;  // Time since last shot in seconds
    float shoot_chance;     // Probability of shooting when cooldown ready (0.0-1.0)
} EnemyWeaponComponent;

/*
 * Player State Component
 */

typedef struct PlayerStateComponent {
    bool  is_alive;
    bool  is_invincible;
    float invincibility_timer;
    float respawn_delay;
} PlayerStateComponent;

/*
 * Functions
 */

void  init_ecs();
void  update_ecs_system_callbacks(void);
void* add_component_impl(ecs_id_t entity, ecs_id_t component_id, void* args);
