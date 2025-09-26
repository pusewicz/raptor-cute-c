#include "ecs.h"

#include "../engine/game_state.h"
#include "factories.h"

#define PICO_ECS_IMPLEMENTATION

#include <cute_array.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_input.h>
#include <cute_math.h>
#include <cute_rnd.h>
#include <cute_sprite.h>
#include <pico_ecs.h>
#include <stddef.h>

/*
 * System update functions
 */

static ecs_ret_t collision_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)udata;

  dyna ecs_id_t *entities_to_destroy = nullptr;

  for (int i = 0; i < entity_count; i++) {
    for (int j = i + 1; j < entity_count; j++) {
      ecs_id_t entity_a = entities[i];
      ecs_id_t entity_b = entities[j];

      if (!ecs_is_entity_ready(ecs, entity_a) || !ecs_is_entity_ready(ecs, entity_b)) {
        continue;
      }

      PositionComponent *pos_a = ECS_GET(entity_a, PositionComponent);
      PositionComponent *pos_b = ECS_GET(entity_b, PositionComponent);
      ColliderComponent *col_a = ECS_GET(entity_a, ColliderComponent);
      ColliderComponent *col_b = ECS_GET(entity_b, ColliderComponent);

      CF_Aabb aabb_a = cf_make_aabb_center_half_extents(*pos_a, col_a->half_extents);
      CF_Aabb aabb_b = cf_make_aabb_center_half_extents(*pos_b, col_b->half_extents);

      if (cf_aabb_to_aabb(aabb_a, aabb_b)) {
        apush(entities_to_destroy, entity_a);
        apush(entities_to_destroy, entity_b);
      }
    }
  }

  if (entities_to_destroy) {
    for (int i = 0; i < asize(entities_to_destroy); ++i) {
      if (ecs_is_entity_ready(ecs, entities_to_destroy[i])) {
        TagComponent *tag = ECS_GET(entities_to_destroy[i], TagComponent);
        if (*tag == TAG_ENEMY) {
          EnemySpawnComponent *spawn = ECS_GET(g_state->enemy_spawner_entity, EnemySpawnComponent);
          --spawn->current_enemy_count;
        }
        ecs_queue_destroy(ecs, entities_to_destroy[i]);
      }
    }

    afree(entities_to_destroy);
  }

  return 0;
}

/*
 * Boundary system ensures entities stay within the canvas bounds.
 *
 * This system checks if an entity's collider is outside the canvas bounds and
 * moves it back within the bounds if necessary.
 *
 * If the entity is an enemy or a bullet, it destroys the entity.
 */
static ecs_ret_t boundary_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)udata;

  for (int i = 0; i < entity_count; ++i) {
    ecs_id_t           entity_id = entities[i];
    ColliderComponent *collider  = ECS_GET(entity_id, ColliderComponent);
    PositionComponent *position  = ECS_GET(entity_id, PositionComponent);
    CF_Aabb canvas_aabb = cf_make_aabb_center_half_extents(cf_v2(0, 0), cf_div_v2_f(g_state->canvas_size, 2.0f));
    CF_Aabb entity_aabb = cf_make_aabb_center_half_extents(*position, collider->half_extents);

    // Check if entity is outside canvas bounds
    if (!cf_aabb_to_aabb(canvas_aabb, entity_aabb)) {
      TagComponent *tag = ECS_GET(entity_id, TagComponent);
      switch (*tag) {
        case TAG_BULLET:
        case TAG_ENEMY:
          // TODO: Add add/remove callbacks to the spawn system to always keep track of the number of enemies
          if (*tag == TAG_ENEMY) {
            EnemySpawnComponent *spawn = ECS_GET(g_state->enemy_spawner_entity, EnemySpawnComponent);
            --spawn->current_enemy_count;
          }
          ecs_queue_destroy(ecs, entity_id);
          break;
        default:
          break;
      }
    }
  }

  return 0;
}

static ecs_ret_t
debug_bounding_boxes_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  if (!g_state->debug_bounding_boxes) {
    return 0;
  }

  (void)ecs;
  (void)dt;
  (void)udata;

  for (int i = 0; i < entity_count; ++i) {
    ecs_id_t entity_id = entities[i];

    SpriteComponent   *sprite   = ECS_GET(entity_id, SpriteComponent);
    PositionComponent *position = ECS_GET(entity_id, PositionComponent);
    ColliderComponent *collider = ECS_GET(entity_id, ColliderComponent);
    CF_Aabb aabb_sprite   = cf_make_aabb_center_half_extents(*position, cf_v2(sprite->w / 2.0f, sprite->h / 2.0f));
    CF_Aabb aabb_collider = cf_make_aabb_center_half_extents(*position, collider->half_extents);

    cf_draw_push();
    // Draw sprite box in red
    cf_draw_push_color(cf_color_red());
    cf_draw_box(aabb_sprite, 0, 0);
    cf_draw_pop_color();

    // Draw collider box in blue
    cf_draw_push_color(cf_color_blue());
    cf_draw_quad(aabb_collider, 0, 0);
    cf_draw_pop_color();
    cf_draw_pop();
  }

  return 0;
}

static ecs_ret_t input_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)ecs;
  (void)dt;
  (void)entities;
  (void)entity_count;
  (void)udata;

  InputComponent *input = ECS_GET(g_state->player_entity, InputComponent);

  input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
  input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
  input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
  input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
  input->shoot = cf_key_down(CF_KEY_SPACE) || cf_mouse_down(CF_MOUSE_BUTTON_LEFT);

  return 0;
}

static ecs_ret_t movement_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)ecs;
  (void)dt;
  (void)udata;

  // TODO: Special case for player for now
  {
    VelocityComponent *vel   = ECS_GET(g_state->player_entity, VelocityComponent);
    InputComponent    *input = ECS_GET(g_state->player_entity, InputComponent);
    float              speed = 1.0f;
    vel->x                   = 0.0f;
    vel->y                   = 0.0f;
    if (input->up) {
      vel->y += speed;
    }
    if (input->down) {
      vel->y -= speed;
    }
    if (input->left) {
      vel->x -= speed;
    }
    if (input->right) {
      vel->x += speed;
    }
  }

  for (int i = 0; i < entity_count; i++) {
    PositionComponent *pos = ECS_GET(entities[i], PositionComponent);
    VelocityComponent *vel = ECS_GET(entities[i], VelocityComponent);
    pos->x += vel->x;
    pos->y += vel->y;
  }

  return 0;
}

static ecs_ret_t render_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)ecs;
  (void)dt;
  (void)udata;

  for (int i = 0; i < entity_count; i++) {
    PositionComponent *pos    = ECS_GET(entities[i], PositionComponent);
    SpriteComponent   *sprite = ECS_GET(entities[i], SpriteComponent);

    cf_sprite_update(sprite);
    cf_draw_push();
    cf_draw_translate_v2(*pos);
    cf_draw_sprite(sprite);
    cf_draw_pop();
  }

  return 0;
}

// TODO: Replace with a coroutine system so it's easier to design spawn patterns
static ecs_ret_t enemy_spawn_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)ecs;
  (void)entities;
  (void)entity_count;
  (void)dt;
  (void)udata;

  EnemySpawnComponent *spawn = ECS_GET(g_state->enemy_spawner_entity, EnemySpawnComponent);
  spawn->time_since_last_spawn += (float)dt;

  if (spawn->time_since_last_spawn >= spawn->spawn_interval && spawn->current_enemy_count < spawn->max_enemies) {
    spawn->time_since_last_spawn = 0.0f;
    spawn->current_enemy_count++;

    float x = cf_rnd_range_float(&g_state->rnd, -20.0f, 20.0f);
    float y = g_state->canvas_size.y * 0.1f;
    make_enemy(x, y);
  }

  return 0;
}

static ecs_ret_t weapon_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)ecs;
  (void)entities;
  (void)entity_count;
  (void)udata;

  WeaponComponent *weapon = ECS_GET(g_state->player_entity, WeaponComponent);

  if (weapon->time_since_shot < weapon->cooldown) {
    weapon->time_since_shot += (float)dt;
    return 0;
  }

  InputComponent *input = ECS_GET(g_state->player_entity, InputComponent);
  if (input->shoot) {
    weapon->time_since_shot = 0.0f;
    PositionComponent *pos  = ECS_GET(g_state->player_entity, PositionComponent);
    make_bullet(pos->x, pos->y, cf_v2(0, 1));
  }

  return 0;
}

void init_ecs(void) {
  ECS_REGISTER_COMPONENT(ColliderComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(EnemySpawnComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(InputComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(PositionComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(SpriteComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(VelocityComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(WeaponComponent, nullptr, nullptr);
  ECS_REGISTER_COMPONENT(TagComponent, nullptr, nullptr);

  ECS_REGISTER_SYSTEM(boundary, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(collision, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(debug_bounding_boxes, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(enemy_spawn, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(input, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(movement, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(render, nullptr, nullptr, nullptr);
  ECS_REGISTER_SYSTEM(weapon, nullptr, nullptr, nullptr);

  // Define which components each system operates on
  ECS_REQUIRE_COMPONENT(boundary, PositionComponent, TagComponent);
  ECS_REQUIRE_COMPONENT(collision, ColliderComponent, PositionComponent, TagComponent);
  ECS_REQUIRE_COMPONENT(input, InputComponent, PositionComponent, TagComponent);
  ECS_REQUIRE_COMPONENT(movement, PositionComponent, VelocityComponent);
  ECS_REQUIRE_COMPONENT(weapon, WeaponComponent, InputComponent, PositionComponent);
  ECS_REQUIRE_COMPONENT(enemy_spawn, EnemySpawnComponent, TagComponent);
  ECS_REQUIRE_COMPONENT(render, PositionComponent, SpriteComponent);
  ECS_REQUIRE_COMPONENT(debug_bounding_boxes, PositionComponent, ColliderComponent, SpriteComponent);
}

// Set the update functions for each system
//
// This function should be called on hot-reload to ensure the latest code is used.
void update_system_callbacks(void) {
  ECS_SET_SYSTEM_CALLBACKS(boundary);
  ECS_SET_SYSTEM_CALLBACKS(collision);
  ECS_SET_SYSTEM_CALLBACKS(debug_bounding_boxes);
  ECS_SET_SYSTEM_CALLBACKS(enemy_spawn);
  ECS_SET_SYSTEM_CALLBACKS(input);
  ECS_SET_SYSTEM_CALLBACKS(movement);
  ECS_SET_SYSTEM_CALLBACKS(render);
  ECS_SET_SYSTEM_CALLBACKS(weapon);
}
