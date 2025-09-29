#pragma once

#include <cute_math.h>
#include <cute_sprite.h>
#include <stddef.h>

#include "pico_ecs.h"

/*
 * Convenience macros for ECS operations
 */

// Get a component of type T from an entity
#define ECS_GET(entity, T) (T*)ecs_get(g_state->ecs, entity, g_state->components.COMPONENT_TO_UNDERSCORE(T))

#define ECS_REGISTER_COMPONENT(T, ...)          g_state->components.COMPONENT_TO_UNDERSCORE(T) = ecs_register_component(g_state->ecs, sizeof(T), __VA_ARGS__)
#define ECS_REGISTER_SYSTEM(name, ...)          g_state->systems.name = ecs_register_system(g_state->ecs, name##_system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_EACH(system, T)   ecs_require_component(g_state->ecs, g_state->systems.system, g_state->components.COMPONENT_TO_UNDERSCORE(T));
#define ECS_REQUIRE_COMPONENT_1(system, T)      ECS_REQUIRE_COMPONENT_EACH(system, T)
#define ECS_REQUIRE_COMPONENT_2(system, T, ...) ECS_REQUIRE_COMPONENT_EACH(system, T) ECS_REQUIRE_COMPONENT_1(system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_3(system, T, ...) ECS_REQUIRE_COMPONENT_EACH(system, T) ECS_REQUIRE_COMPONENT_2(system, __VA_ARGS__)
#define ECS_REQUIRE_COMPONENT_4(system, T, ...) ECS_REQUIRE_COMPONENT_EACH(system, T) ECS_REQUIRE_COMPONENT_3(system, __VA_ARGS__)

#define ECS_REQUIRE_COMPONENT_GET_MACRO(_1, _2, _3, _4, NAME, ...) NAME
#define ECS_REQUIRE_COMPONENT(system, ...) \
    ECS_REQUIRE_COMPONENT_GET_MACRO(__VA_ARGS__, ECS_REQUIRE_COMPONENT_4, ECS_REQUIRE_COMPONENT_3, ECS_REQUIRE_COMPONENT_2, ECS_REQUIRE_COMPONENT_1)(system, __VA_ARGS__)
#define ECS_UPDATE_SYSTEM(name)        ecs_update_system(g_state->ecs, g_state->systems.name, CF_DELTA_TIME)
#define ECS_SET_SYSTEM_CALLBACKS(name) ecs_set_system_callbacks(g_state->ecs, g_state->systems.name, name##_system, nullptr, nullptr)

// Add a component to an entity
#define ECS_ADD_COMPONENT(entity, T) ((T*)add_component_impl(entity, g_state->components.COMPONENT_TO_UNDERSCORE(T), nullptr))

// Helper macros to convert CamelCase to snake_case for component names
#define COMPONENT_TO_UNDERSCORE(T)           UNDERSCORE_##T
#define UNDERSCORE_BulletComponent           bullet
#define UNDERSCORE_BackgroundScrollComponent background_scroll
#define UNDERSCORE_ColliderComponent         collider
#define UNDERSCORE_EnemySpawnComponent       enemy_spawn
#define UNDERSCORE_InputComponent            input
#define UNDERSCORE_PositionComponent         position
#define UNDERSCORE_SpriteComponent           sprite
#define UNDERSCORE_TagComponent              tag
#define UNDERSCORE_VelocityComponent         velocity
#define UNDERSCORE_WeaponComponent           weapon

// Convenience macro to create a new entity
#define make_entity() ecs_create(g_state->ecs)

/*
 * Component structures
 */

#define BACKGROUND_SCROLL_SPRITE_COUNT 6 * 3
typedef struct BackgroundScrollComponent {
    CF_Sprite sprites[BACKGROUND_SCROLL_SPRITE_COUNT];  // Background sprite
    float     y_offset;                                 // Vertical offset for scrolling
    float     max_y_offset;                             // Maximum offset before resetting
} BackgroundScrollComponent;

typedef struct BulletComponent {
    CF_V2 direction;
} BulletComponent;

typedef struct ColliderComponent {
    CF_V2 half_extents;
} ColliderComponent;

typedef struct EnemySpawnComponent {
    float spawn_interval;         // Time between spawns in seconds
    float time_since_last_spawn;  // Time since last spawn in seconds
    int   max_enemies;            // Maximum number of enemies allowed
    int   current_enemy_count;    // Current number of enemies spawned
} EnemySpawnComponent;

typedef struct InputComponent {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
} InputComponent;

typedef CF_V2     PositionComponent;
typedef CF_Sprite SpriteComponent;

typedef enum TagComponent { TAG_PLAYER, TAG_ENEMY, TAG_BULLET } TagComponent;

typedef CF_V2 VelocityComponent;

typedef struct WeaponComponent {
    float cooldown;         // Time between shots in seconds
    float time_since_shot;  // Time since last shot in seconds
} WeaponComponent;

/*
 * Components
 */
typedef struct {
    ecs_id_t background_scroll;
    ecs_id_t collider;
    ecs_id_t enemy_spawn;
    ecs_id_t input;
    ecs_id_t position;
    ecs_id_t sprite;
    ecs_id_t velocity;
    ecs_id_t weapon;
    ecs_id_t tag;
} Components;

/*
 * Systems
 */
typedef struct {
    ecs_id_t background_scroll;
    ecs_id_t boundary;
    ecs_id_t collision;
    ecs_id_t debug_bounding_boxes;
    ecs_id_t enemy_spawn;
    ecs_id_t input;
    ecs_id_t movement;
    ecs_id_t render;
    ecs_id_t weapon;
} Systems;

void  init_ecs(size_t entity_count);
void  update_ecs_system_callbacks(void);
void* add_component_impl(ecs_id_t entity, ecs_id_t component_id, void* args);
