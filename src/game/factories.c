#include "factories.h"

#include "../engine/game_state.h"
#include "ecs.h"

#include <SDL3/SDL_log.h>
#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>
#include <stdbool.h>
#include <stddef.h>

ecs_id_t make_player(float x, float y) {
  ecs_id_t id = ecs_create(g_state->ecs);

  // Add position
  CF_V2 *pos = ecs_add(g_state->ecs, id, g_state->components.position, NULL);
  pos->x     = x;
  pos->y     = y;

  // Add velocity
  CF_V2 *vel = ecs_add(g_state->ecs, id, g_state->components.velocity, NULL);
  vel->x     = 0.0f;
  vel->y     = 0.0f;

  // Add input controls
  InputComponent *input = ecs_add(g_state->ecs, id, g_state->components.input, NULL);
  input->up             = false;
  input->down           = false;
  input->left           = false;
  input->right          = false;

  // Add sprite
  CF_Sprite *sprite = ecs_add(g_state->ecs, id, g_state->components.sprite, NULL);
  CF_Result  result;
  *sprite = cf_make_easy_sprite_from_png("assets/player.png", &result);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load player sprite: %s\n", result.details);
  }

  // Add weapon
  WeaponComponent *weapon = ecs_add(g_state->ecs, id, g_state->components.weapon, NULL);
  weapon->cooldown        = 0.5f;    // Half a second between shots
  weapon->time_since_shot = 0.0f;

  return id;
}

ecs_id_t make_bullet(float x, float y, CF_V2 direction) {
  ecs_id_t id = ecs_create(g_state->ecs);

  // Add bullet component
  BulletComponent *bullet = ecs_add(g_state->ecs, id, g_state->components.bullet, NULL);
  bullet->direction       = direction;

  // Add position
  CF_V2 *pos = ecs_add(g_state->ecs, id, g_state->components.position, NULL);
  pos->x     = x;
  pos->y     = y;

  // Add velocity
  CF_V2 *vel = ecs_add(g_state->ecs, id, g_state->components.velocity, NULL);
  vel->x     = 0.0f;
  vel->y     = 1 == direction.y ? 2.0f : -2.0f;

  // Add sprite
  CF_Sprite *sprite = ecs_add(g_state->ecs, id, g_state->components.sprite, NULL);
  CF_Result  result;
  *sprite = cf_make_easy_sprite_from_png("assets/bullet.png", &result);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load bullet sprite: %s\n", result.details);
  }

  // Add collider
  ColliderComponent *collider = ecs_add(g_state->ecs, id, g_state->components.collider, NULL);
  collider->half_extents      = cf_v2(sprite->w / 2.0, sprite->h / 2.0);

  return id;
}

ecs_id_t make_enemy(float x, float y) {
  ecs_id_t id = ecs_create(g_state->ecs);

  // Add position
  CF_V2 *pos = ecs_add(g_state->ecs, id, g_state->components.position, NULL);
  pos->x     = x;
  pos->y     = y;

  // Add velocity
  CF_V2 *vel = ecs_add(g_state->ecs, id, g_state->components.velocity, NULL);
  vel->x     = 0.0f;
  vel->y     = -0.1f;

  // Add sprite
  CF_Sprite *sprite = ecs_add(g_state->ecs, id, g_state->components.sprite, NULL);
  *sprite           = cf_make_sprite("assets/alan.ase");

  // Add collider
  ColliderComponent *collider = ecs_add(g_state->ecs, id, g_state->components.collider, NULL);
  collider->half_extents      = cf_v2(sprite->w / 2.0, sprite->h / 2.0);

  return id;
}

ecs_id_t make_enemy_spawner(void) {
  ecs_id_t id = ecs_create(g_state->ecs);

  EnemySpawnComponent *spawn   = ecs_add(g_state->ecs, id, g_state->components.enemy_spawn, NULL);
  spawn->spawn_interval        = 5.0f;    // Spawn an enemy every 5 seconds
  spawn->time_since_last_spawn = 0.0f;
  spawn->max_enemies           = 5;
  spawn->current_enemy_count   = 0;

  return id;
}
