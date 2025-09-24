#include "ecs.h"

#include "../engine/game_state.h"
#include "../engine/log.h"
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

      CF_V2             *pos_a = ecs_get(ecs, entity_a, g_state->components.position);
      CF_V2             *pos_b = ecs_get(ecs, entity_b, g_state->components.position);
      ColliderComponent *col_a = ecs_get(ecs, entity_a, g_state->components.collider);
      ColliderComponent *col_b = ecs_get(ecs, entity_b, g_state->components.collider);

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
        TagType *tag = ecs_get(ecs, entities_to_destroy[i], g_state->components.tag);
        if (*tag == TAG_ENEMY) {
          EnemySpawnComponent *spawn = ecs_get(ecs, g_state->enemy_spawner_entity, g_state->components.enemy_spawn);
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
    ColliderComponent *collider  = ecs_get(ecs, entity_id, g_state->components.collider);
    CF_V2             *position  = ecs_get(ecs, entity_id, g_state->components.position);
    CF_Aabb canvas_aabb = cf_make_aabb_center_half_extents(cf_v2(0, 0), cf_div_v2_f(g_state->canvas_size, 2.0f));
    CF_Aabb entity_aabb = cf_make_aabb_center_half_extents(*position, collider->half_extents);

    // Check if entity is outside canvas bounds
    if (!cf_aabb_to_aabb(canvas_aabb, entity_aabb)) {
      APP_DEBUG("Entity %d is outside canvas bounds", entity_id);
      TagComponent *tag = ecs_get(ecs, entity_id, g_state->components.tag);
      switch (tag->tag) {
        case TAG_ENEMY:
        case TAG_BULLET: {
          // TODO: Add add/remove callbacks to the spawn system to always keep track of the number of enemies
          if (tag->tag == TAG_ENEMY) {
            EnemySpawnComponent *spawn = ecs_get(ecs, g_state->systems.enemy_spawn, g_state->components.enemy_spawn);
            --spawn->current_enemy_count;
          }
          ecs_queue_destroy(ecs, entity_id);
          break;
        }
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

  (void)dt;
  (void)udata;

  for (int i = 0; i < entity_count; ++i) {
    ecs_id_t entity_id = entities[i];

    CF_Sprite         *sprite      = ecs_get(ecs, entity_id, g_state->components.sprite);
    CF_V2             *pos         = ecs_get(ecs, entity_id, g_state->components.position);
    ColliderComponent *col         = ecs_get(ecs, entity_id, g_state->components.collider);
    CF_Aabb            aabb_sprite = cf_make_aabb_center_half_extents(*pos, cf_v2(sprite->w / 2.0f, sprite->h / 2.0f));
    CF_Aabb            aabb_collider = cf_make_aabb_center_half_extents(*pos, col->half_extents);

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
  (void)dt;
  (void)entities;
  (void)entity_count;
  (void)udata;

  InputComponent *input = ecs_get(ecs, g_state->player_entity, g_state->components.input);

  input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
  input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
  input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
  input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
  input->shoot = cf_key_down(CF_KEY_SPACE) || cf_mouse_down(CF_MOUSE_BUTTON_LEFT);

  return 0;
}

static ecs_ret_t movement_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)udata;

  // TODO: Special case for player for now
  {
    CF_V2          *vel   = ecs_get(ecs, g_state->player_entity, g_state->components.velocity);
    InputComponent *input = ecs_get(ecs, g_state->player_entity, g_state->components.input);
    float           speed = 1.0f;
    vel->x                = 0.0f;
    vel->y                = 0.0f;
    if (input->up)
      vel->y += speed;
    if (input->down)
      vel->y -= speed;
    if (input->left)
      vel->x -= speed;
    if (input->right)
      vel->x += speed;
  }

  for (int i = 0; i < entity_count; i++) {
    CF_V2 *pos = ecs_get(ecs, entities[i], g_state->components.position);
    CF_V2 *vel = ecs_get(ecs, entities[i], g_state->components.velocity);
    pos->x += vel->x;
    pos->y += vel->y;
  }

  return 0;
}

static ecs_ret_t render_system(ecs_t *ecs, ecs_id_t *entities, int entity_count, ecs_dt_t dt, void *udata) {
  (void)dt;
  (void)udata;

  for (int i = 0; i < entity_count; i++) {
    CF_V2     *pos    = ecs_get(ecs, entities[i], g_state->components.position);
    CF_Sprite *sprite = ecs_get(ecs, entities[i], g_state->components.sprite);

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
  (void)entities;
  (void)entity_count;
  (void)dt;
  (void)udata;

  EnemySpawnComponent *spawn = ecs_get(ecs, g_state->enemy_spawner_entity, g_state->components.enemy_spawn);
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
  (void)entities;
  (void)entity_count;
  (void)udata;

  WeaponComponent *weapon = ecs_get(ecs, g_state->player_entity, g_state->components.weapon);

  if (weapon->time_since_shot < weapon->cooldown) {
    weapon->time_since_shot += (float)dt;
    return 0;
  }

  InputComponent *input = ecs_get(ecs, g_state->player_entity, g_state->components.input);
  if (input->shoot) {
    weapon->time_since_shot = 0.0f;
    CF_V2 *pos              = ecs_get(ecs, g_state->player_entity, g_state->components.position);
    make_bullet(pos->x, pos->y, cf_v2(0, 1));
  }

  return 0;
}

void register_components(void) {
  g_state->components.collider    = ecs_register_component(g_state->ecs, sizeof(ColliderComponent), nullptr, nullptr);
  g_state->components.enemy_spawn = ecs_register_component(g_state->ecs, sizeof(EnemySpawnComponent), nullptr, nullptr);
  g_state->components.input       = ecs_register_component(g_state->ecs, sizeof(InputComponent), nullptr, nullptr);
  g_state->components.position    = ecs_register_component(g_state->ecs, sizeof(CF_V2), nullptr, nullptr);
  g_state->components.sprite      = ecs_register_component(g_state->ecs, sizeof(CF_Sprite), nullptr, nullptr);
  g_state->components.velocity    = ecs_register_component(g_state->ecs, sizeof(CF_V2), nullptr, nullptr);
  g_state->components.weapon      = ecs_register_component(g_state->ecs, sizeof(WeaponComponent), nullptr, nullptr);
  g_state->components.tag         = ecs_register_component(g_state->ecs, sizeof(TagType), nullptr, nullptr);
}

void register_systems(void) {
  g_state->systems.boundary    = ecs_register_system(g_state->ecs, boundary_system, nullptr, nullptr, nullptr);
  g_state->systems.collision   = ecs_register_system(g_state->ecs, collision_system, nullptr, nullptr, nullptr);
  g_state->systems.debug_bounding_boxes   = ecs_register_system(g_state->ecs, debug_bounding_boxes_system, nullptr, nullptr, nullptr);
  g_state->systems.enemy_spawn = ecs_register_system(g_state->ecs, enemy_spawn_system, nullptr, nullptr, nullptr);
  g_state->systems.input       = ecs_register_system(g_state->ecs, input_system, nullptr, nullptr, nullptr);
  g_state->systems.movement    = ecs_register_system(g_state->ecs, movement_system, nullptr, nullptr, nullptr);
  g_state->systems.render      = ecs_register_system(g_state->ecs, render_system, nullptr, nullptr, nullptr);
  g_state->systems.weapon      = ecs_register_system(g_state->ecs, weapon_system, nullptr, nullptr, nullptr);

  /*
   * Boundary system requires position and tag components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.boundary, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.boundary, g_state->components.tag);

  /*
   * Collision system requires collider, position, and tag components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.collision, g_state->components.collider);
  ecs_require_component(g_state->ecs, g_state->systems.collision, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.collision, g_state->components.tag);

  /*
   * Input system requires input, position, and tag components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.input, g_state->components.input);
  ecs_require_component(g_state->ecs, g_state->systems.input, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.collision, g_state->components.tag);

  /*
   * Movement system requires position and velocity components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.movement, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.movement, g_state->components.velocity);

  /*
   * Weapon system requires weapon, input, and position components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.weapon, g_state->components.weapon);
  ecs_require_component(g_state->ecs, g_state->systems.weapon, g_state->components.input);
  ecs_require_component(g_state->ecs, g_state->systems.weapon, g_state->components.position);

  /*
   * Enemy spawn system requires enemy spawn and tag components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.enemy_spawn, g_state->components.enemy_spawn);
  ecs_require_component(g_state->ecs, g_state->systems.enemy_spawn, g_state->components.tag);

  /*
   * Render system requires position and sprite components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.render, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.render, g_state->components.sprite);

  /*
   * Debug system requires position, collider, and sprite components.
   */
  ecs_require_component(g_state->ecs, g_state->systems.debug_bounding_boxes, g_state->components.position);
  ecs_require_component(g_state->ecs, g_state->systems.debug_bounding_boxes, g_state->components.collider);
  ecs_require_component(g_state->ecs, g_state->systems.debug_bounding_boxes, g_state->components.sprite);
}

void update_system_callbacks(void) {
  ecs_t *ecs = g_state->ecs;

  ecs_set_system_callbacks(ecs, g_state->systems.boundary, boundary_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.collision, collision_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.debug_bounding_boxes, debug_bounding_boxes_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.input, input_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.movement, movement_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.render, render_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.weapon, weapon_system, nullptr, nullptr);
  ecs_set_system_callbacks(ecs, g_state->systems.enemy_spawn, enemy_spawn_system, nullptr, nullptr);
}
