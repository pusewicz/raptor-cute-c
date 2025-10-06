#include "ecs.h"

#include <dcimgui.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"
#include "asset/audio.h"
#include "asset/sprite.h"
#include "factory.h"

#define PICO_ECS_IMPLEMENTATION

#include <cute_audio.h>
#include <cute_color.h>
#include <cute_coroutine.h>
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

static ecs_ret_t background_scroll_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    auto bg_scroll = ECS_GET(g_state->entities.background_scroll, BackgroundScrollComponent);
    bg_scroll->y_offset += 0.1f;
    if (bg_scroll->y_offset >= bg_scroll->max_y_offset) { bg_scroll->y_offset = 0; }

    {
        cf_draw() {
            cf_draw_translate(0, g_state->canvas_size.y / 2.0f - bg_scroll->y_offset + bg_scroll->max_y_offset * 0.5f);
            int i = 0;
            for (int y = 0; y < (BACKGROUND_SCROLL_SPRITE_COUNT / 3); ++y) {
                for (int x = -1; x <= 1; ++x) {
                    CF_Sprite* sprite = &bg_scroll->sprites[i];
                    cf_draw() {
                        cf_draw_translate(x * sprite->w, -y * sprite->h);
                        cf_sprite_update(sprite);
                        cf_sprite_draw(sprite);
                    }
                    ++i;
                }
            }
        }
    }

    return 0;
}

static ecs_ret_t collision_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities,
    int       entity_count,
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    for (int i = 0; i < entity_count; ++i) {
        if (!ECS_READY(entities[i])) { continue; }
        for (int j = i + 1; j < entity_count; ++j) {
            if (!ECS_READY(entities[j]) || entities[i] == entities[j]) { continue; }

            auto pos_a  = ECS_GET(entities[i], PositionComponent);
            auto pos_b  = ECS_GET(entities[j], PositionComponent);
            auto col_a  = ECS_GET(entities[i], ColliderComponent);
            auto col_b  = ECS_GET(entities[j], ColliderComponent);
            auto aabb_a = cf_make_aabb_center_half_extents(*pos_a, col_a->half_extents);
            auto aabb_b = cf_make_aabb_center_half_extents(*pos_b, col_b->half_extents);

            if (cf_aabb_to_aabb(aabb_a, aabb_b)) {
                auto tag_a = ECS_GET(entities[i], TagComponent);
                auto tag_b = ECS_GET(entities[j], TagComponent);

                // Bullet vs Enemy collision
                if ((*tag_a == TAG_BULLET && *tag_b == TAG_ENEMY) || (*tag_a == TAG_ENEMY && *tag_b == TAG_BULLET)) {
                    // Add score
                    auto score = ECS_GET((*tag_a == TAG_ENEMY) ? entities[i] : entities[j], ScoreComponent);
                    g_state->score += *score;

                    // Create explosion
                    auto explosion_pos = (*tag_a == TAG_BULLET) ? pos_b : pos_a;
                    make_explosion(explosion_pos->x, explosion_pos->y);
                    cf_play_sound(g_state->audio.explosion, cf_sound_params_defaults());

                    // Destroy both entities
                    ECS_QUEUE_DESTROY(entities[i]);
                    ECS_QUEUE_DESTROY(entities[j]);
                }
                // Player vs Enemy collision
                else if ((*tag_a == TAG_PLAYER && *tag_b == TAG_ENEMY) ||
                         (*tag_a == TAG_ENEMY && *tag_b == TAG_PLAYER)) {
                    auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);

                    // Only damage if player is alive and not invincible
                    if (state->is_alive && !state->is_invincible) {
                        // Destroy the enemy
                        ecs_id_t enemy_id = (*tag_a == TAG_ENEMY) ? entities[i] : entities[j];
                        ECS_QUEUE_DESTROY(enemy_id);

                        // Decrement lives
                        g_state->lives--;

                        // Create explosion at player position
                        auto player_pos = ECS_GET(g_state->entities.player, PositionComponent);
                        make_explosion(player_pos->x, player_pos->y);
                        cf_play_sound(g_state->audio.explosion, cf_sound_params_defaults());

                        // Mark player as dead
                        state->is_alive      = false;
                        state->is_invincible = false;

                        // Set respawn delay if player has lives remaining
                        if (g_state->lives > 0) {
                            state->respawn_delay = 2.0f;  // 2 second respawn delay
                        }
                    }
                }
            }
        }
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
static ecs_ret_t boundary_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities,
    int       entity_count,
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    for (int i = 0; i < entity_count; ++i) {
        auto entity_id   = entities[i];
        auto collider    = ECS_GET(entity_id, ColliderComponent);
        auto position    = ECS_GET(entity_id, PositionComponent);
        auto canvas_aabb = cf_make_aabb_center_half_extents(cf_v2(0, 0), cf_div_v2_f(g_state->canvas_size, 2.0f));
        auto entity_aabb = cf_make_aabb_center_half_extents(*position, collider->half_extents);

        // Check if entity is outside canvas bounds
        if (!cf_aabb_to_aabb(canvas_aabb, entity_aabb)) {
            auto tag = ECS_GET(entity_id, TagComponent);
            switch (*tag) {
                case TAG_BULLET:
                case TAG_ENEMY:
                    ECS_QUEUE_DESTROY(entity_id);
                    break;
                default:
                    break;
            }
        }
    }

    return 0;
}

static ecs_ret_t coroutine_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    if (cf_coroutine_state(g_state->coroutines.spawner) != CF_COROUTINE_STATE_DEAD) {
        cf_coroutine_resume(g_state->coroutines.spawner);
    }

    return 0;
}

static ecs_ret_t debug_bounding_boxes_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities,
    int       entity_count,
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    if (!g_state->debug_bounding_boxes) { return 0; }

    for (int i = 0; i < entity_count; ++i) {
        ecs_id_t entity_id = entities[i];

        auto sprite        = ECS_GET(entity_id, SpriteComponent);
        auto position      = ECS_GET(entity_id, PositionComponent);
        auto collider      = ECS_GET(entity_id, ColliderComponent);
        auto aabb_sprite =
            cf_make_aabb_center_half_extents(*position, cf_v2(sprite->sprite.w / 2.0f, sprite->sprite.h / 2.0f));
        auto aabb_collider = cf_make_aabb_center_half_extents(*position, collider->half_extents);

        cf_draw() {
            // Draw sprite box in red
            cf_draw_color(cf_color_red()) { cf_draw_box(aabb_sprite, 0, 0); }

            // Draw collider box in blue
            cf_draw_color(cf_color_blue()) { cf_draw_quad(aabb_collider, 0, 0); }
        }
    }

    return 0;
}

static ecs_ret_t input_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);
    auto input = ECS_GET(g_state->entities.player, InputComponent);

    // Disable input if player is dead
    if (!state->is_alive) {
        input->up    = false;
        input->down  = false;
        input->left  = false;
        input->right = false;
        input->shoot = false;
        return 0;
    }

    input->up    = cf_key_down(CF_KEY_W) || cf_key_down(CF_KEY_UP);
    input->down  = cf_key_down(CF_KEY_S) || cf_key_down(CF_KEY_DOWN);
    input->left  = cf_key_down(CF_KEY_A) || cf_key_down(CF_KEY_LEFT);
    input->right = cf_key_down(CF_KEY_D) || cf_key_down(CF_KEY_RIGHT);
    input->shoot = cf_key_down(CF_KEY_SPACE) || cf_mouse_down(CF_MOUSE_BUTTON_LEFT);

    return 0;
}

static ecs_ret_t movement_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities,
    int       entity_count,
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    // TODO: Special case for player for now
    {
        auto  vel   = ECS_GET(g_state->entities.player, VelocityComponent);
        auto  input = ECS_GET(g_state->entities.player, InputComponent);
        float speed = 1.0f;

        vel->x = vel->y = 0.0f;

        if (input->up) vel->y += speed;
        if (input->down) vel->y -= speed;
        if (input->left) vel->x -= speed;
        if (input->right) vel->x += speed;
    }

    for (int i = 0; i < entity_count; i++) {
        auto pos = ECS_GET(entities[i], PositionComponent);
        auto vel = ECS_GET(entities[i], VelocityComponent);

        pos->x += vel->x;
        pos->y += vel->y;

        // Clamp player position to canvas bounds
        if (entities[i] == g_state->entities.player) {
            pos->x = cf_clamp(pos->x, -g_state->canvas_size.x / 2.0f, g_state->canvas_size.x / 2.0f);
            pos->y = cf_clamp(pos->y, -g_state->canvas_size.y / 2.0f, g_state->canvas_size.y / 2.0f);
        }
    }

    return 0;
}

static ecs_ret_t player_state_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt,
    void*     udata [[maybe_unused]]
) {
    auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);

    // Handle respawn delay
    if (!state->is_alive && state->respawn_delay > 0.0f) {
        state->respawn_delay -= (float)dt;
        if (state->respawn_delay <= 0.0f) {
            // Respawn player
            state->is_alive            = true;
            state->is_invincible       = true;
            state->invincibility_timer = 3.0f;  // 3 seconds of invincibility

            // Reset player position
            auto pos                   = ECS_GET(g_state->entities.player, PositionComponent);
            pos->x                     = 0.0f;
            pos->y                     = -g_state->canvas_size.y / 3;
        }
        return 0;
    }

    // Handle invincibility timer
    if (state->is_invincible && state->invincibility_timer > 0.0f) {
        state->invincibility_timer -= (float)dt;
        if (state->invincibility_timer <= 0.0f) { state->is_invincible = false; }
    }

    return 0;
}

static ecs_ret_t player_render_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);

    // Don't render if player is dead
    if (!state->is_alive) { return 0; }

    auto vel    = ECS_GET(g_state->entities.player, VelocityComponent);
    auto pos    = ECS_GET(g_state->entities.player, PositionComponent);
    auto sprite = ECS_GET(g_state->entities.player, PlayerSpriteComponent);

    if (vel->x > 0) {
        if (!cf_sprite_is_playing(&sprite->sprite, "right")) {
            cf_sprite_play(&sprite->sprite, "right");
            cf_sprite_play(&sprite->booster_sprite, "right");
        }
    } else if (vel->x < 0) {
        if (!cf_sprite_is_playing(&sprite->sprite, "left")) {
            cf_sprite_play(&sprite->sprite, "left");
            cf_sprite_play(&sprite->booster_sprite, "left");
        }
    } else if (!cf_sprite_is_playing(&sprite->sprite, "default")) {
        cf_sprite_play(&sprite->sprite, "default");
        cf_sprite_play(&sprite->booster_sprite, "default");
    }

    cf_sprite_update(&sprite->sprite);
    cf_sprite_update(&sprite->booster_sprite);

    // Flicker effect during invincibility
    bool should_render = true;
    if (state->is_invincible) {
        // Flicker every 0.1 seconds
        should_render = ((int)(state->invincibility_timer * 10) % 2) == 0;
    }

    if (should_render) {
        cf_draw() {
            cf_draw_layer(sprite->z_index) {
                cf_draw_translate_v2(*pos);
                cf_draw_sprite(&sprite->sprite);
                cf_draw_sprite(&sprite->booster_sprite);
            }
        }
    }

    return 0;
}

static ecs_ret_t render_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities,
    int       entity_count,
    ecs_dt_t  dt [[maybe_unused]],
    void*     udata [[maybe_unused]]
) {
    for (int i = 0; i < entity_count; i++) {
        auto pos    = ECS_GET(entities[i], PositionComponent);
        auto sprite = ECS_GET(entities[i], SpriteComponent);

        // Destroy sprite if animation is not looped and has finished
        if (!cf_sprite_get_loop(&sprite->sprite) && cf_sprite_will_finish(&sprite->sprite)) {
            ECS_QUEUE_DESTROY(entities[i]);
            continue;
        }

        cf_sprite_update(&sprite->sprite);
        cf_draw() {
            cf_draw_layer(sprite->z_index) {
                cf_draw_translate_v2(*pos);
                cf_draw_sprite(&sprite->sprite);
            }
        }
    }

    return 0;
}

static ecs_ret_t weapon_system(
    ecs_t*    ecs [[maybe_unused]],
    ecs_id_t* entities [[maybe_unused]],
    int       entity_count [[maybe_unused]],
    ecs_dt_t  dt,
    void*     udata [[maybe_unused]]
) {
    auto weapon = ECS_GET(g_state->entities.player, WeaponComponent);

    if (weapon->time_since_shot < weapon->cooldown) {
        weapon->time_since_shot += (float)dt;
        return 0;
    }

    auto input = ECS_GET(g_state->entities.player, InputComponent);
    if (input->shoot) {
        weapon->time_since_shot = 0.0f;
        auto pos                = ECS_GET(g_state->entities.player, PositionComponent);

        make_bullet(pos->x, pos->y, cf_v2(0, 1));

        cf_play_sound(g_state->audio.laser_shoot, cf_sound_params_defaults());
    }

    return 0;
}

void* add_component_impl(ecs_id_t entity_id, ecs_id_t component_id, void* args) {
    return ecs_add(g_state->ecs, entity_id, component_id, args);
}

static void init_background_scroll(
    ecs_t* ecs [[maybe_unused]], ecs_id_t entity_id [[maybe_unused]], void* ptr, void* args [[maybe_unused]]
) {
    auto comp = (BackgroundScrollComponent*)ptr;

    for (int i = 0; i < BACKGROUND_SCROLL_SPRITE_COUNT; ++i) {
        load_sprite(&comp->sprites[i], "assets/background.ase");

        // Set the initial frame to 0 or 1 based on the index to create a
        // checkerboard pattern
        cf_sprite_set_frame(&comp->sprites[i], i % 2);
    }

    comp->max_y_offset = comp->sprites[0].h;
}

void init_ecs() {
    ECS_REGISTER_COMPONENT(BackgroundScrollComponent, init_background_scroll, nullptr);
    ECS_REGISTER_COMPONENT(ColliderComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(InputComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(PlayerSpriteComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(PlayerStateComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(PositionComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(ScoreComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(SpriteComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(TagComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(VelocityComponent, nullptr, nullptr);
    ECS_REGISTER_COMPONENT(WeaponComponent, nullptr, nullptr);

    ECS_REGISTER_SYSTEM(background_scroll, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(background_scroll, BackgroundScrollComponent, PositionComponent, VelocityComponent);

    ECS_REGISTER_SYSTEM(boundary, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(boundary, PositionComponent, TagComponent);

    ECS_REGISTER_SYSTEM(collision, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(collision, ColliderComponent, PositionComponent, TagComponent);

    ECS_REGISTER_SYSTEM(coroutine, nullptr, nullptr, nullptr);

    ECS_REGISTER_SYSTEM(debug_bounding_boxes, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(debug_bounding_boxes, PositionComponent, ColliderComponent, SpriteComponent);

    ECS_REGISTER_SYSTEM(input, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(input, InputComponent, PlayerStateComponent, PositionComponent, TagComponent);

    ECS_REGISTER_SYSTEM(movement, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(movement, PositionComponent, VelocityComponent);

    ECS_REGISTER_SYSTEM(weapon, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(weapon, WeaponComponent, InputComponent, PositionComponent);

    ECS_REGISTER_SYSTEM(player_state, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(player_state, PlayerStateComponent);

    ECS_REGISTER_SYSTEM(render, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(render, PositionComponent, SpriteComponent);

    ECS_REGISTER_SYSTEM(player_render, nullptr, nullptr, nullptr);
    ECS_REQUIRE_COMPONENT(player_render, PositionComponent, PlayerSpriteComponent, PlayerStateComponent);
}

// Set the update functions for each system
//
// This function should be called on hot-reload to ensure the latest code is
// used.
void update_ecs_system_callbacks(void) {
    ECS_SET_SYSTEM_CALLBACKS(boundary);
    ECS_SET_SYSTEM_CALLBACKS(background_scroll);
    ECS_SET_SYSTEM_CALLBACKS(collision);
    ECS_SET_SYSTEM_CALLBACKS(coroutine);
    ECS_SET_SYSTEM_CALLBACKS(debug_bounding_boxes);
    ECS_SET_SYSTEM_CALLBACKS(input);
    ECS_SET_SYSTEM_CALLBACKS(movement);
    ECS_SET_SYSTEM_CALLBACKS(player_state);
    ECS_SET_SYSTEM_CALLBACKS(render);
    ECS_SET_SYSTEM_CALLBACKS(player_render);
    ECS_SET_SYSTEM_CALLBACKS(weapon);
}
