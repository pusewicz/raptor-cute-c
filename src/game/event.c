#include "event.h"

#include <cute_audio.h>
#include <cute_c_runtime.h>
#include <cute_math.h>

#include "../engine/game_state.h"
#include "../engine/log.h"
#include "asset/audio.h"
#include "ecs.h"
#include "factory.h"

const char* event_type_to_string(EventType color) {
    switch (color) {
#define X(name, value) \
    case name:         \
        return #name;
        EVENT_LIST
#undef X
        default:
            return "UNKNOWN";
    }
}

void event_register(EventType type, EventCallback callback) {
    CF_ASSERT(type < EVENT_COUNT);
    CF_ASSERT(event_listener_counts[type] < MAX_EVENT_LISTENERS);
    APP_DEBUG("Registering event listener for event type %s", event_type_to_string(type));
    int count                    = event_listener_counts[type];
    event_listeners[type][count] = callback;
    event_listener_counts[type]++;
}

void event_trigger(EventType type, void* data) {
    for (int i = 0; i < event_listener_counts[type]; i++) { event_listeners[type][i](data); }
}

void event_clear_listeners(void) {
    APP_DEBUG("Clearing event listeners...");
    for (int i = 0; i < EVENT_COUNT; i++) {
        for (int j = 0; j < event_listener_counts[i]; j++) { event_listeners[i][j] = nullptr; }
        event_listener_counts[i] = 0;
    }
}

static void on_collision_destroy_bullet_or_enemy(void* data) {
    auto e = (CollisionEvent*)data;
    if (!ECS_READY(e->entity_a) || !ECS_READY(e->entity_b)) { return; }
    auto tag_a = ECS_GET(e->entity_a, TagComponent);
    auto tag_b = ECS_GET(e->entity_b, TagComponent);

    if ((*tag_a == TAG_BULLET && *tag_b == TAG_ENEMY) || (*tag_a == TAG_ENEMY && *tag_b == TAG_BULLET)) {
        auto score = ECS_GET((*tag_a == TAG_ENEMY) ? e->entity_a : e->entity_b, ScoreComponent);
        g_state->score += *score;

        ECS_QUEUE_DESTROY(e->entity_a);
        ECS_QUEUE_DESTROY(e->entity_b);
    }
}

static void on_collision_explosion(void* data) {
    auto e = (CollisionEvent*)data;
    if (!ECS_READY(e->entity_a) || !ECS_READY(e->entity_b)) { return; }
    auto tag_a = ECS_GET(e->entity_a, TagComponent);
    auto tag_b = ECS_GET(e->entity_b, TagComponent);

    if ((*tag_a == TAG_BULLET && *tag_b == TAG_ENEMY) || (*tag_a == TAG_ENEMY && *tag_b == TAG_BULLET)) {
        auto pos_a         = ECS_GET(e->entity_a, PositionComponent);
        auto pos_b         = ECS_GET(e->entity_b, PositionComponent);
        auto explosion_pos = (*tag_a == TAG_BULLET) ? pos_b : pos_a;

        make_explosion(explosion_pos->x, explosion_pos->y);

        cf_play_sound(g_state->audio.explosion, cf_sound_params_defaults());
    }
}

static void on_collision_player_damage(void* data) {
    auto e = (CollisionEvent*)data;
    if (!ECS_READY(e->entity_a) || !ECS_READY(e->entity_b)) { return; }
    auto tag_a = ECS_GET(e->entity_a, TagComponent);
    auto tag_b = ECS_GET(e->entity_b, TagComponent);

    // Check if player collides with enemy
    if ((*tag_a == TAG_PLAYER && *tag_b == TAG_ENEMY) || (*tag_a == TAG_ENEMY && *tag_b == TAG_PLAYER)) {
        auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);

        // Only damage if player is alive and not invincible
        if (state->is_alive && !state->is_invincible) {
            // Destroy the enemy
            ecs_id_t enemy_id = (*tag_a == TAG_ENEMY) ? e->entity_a : e->entity_b;
            ECS_QUEUE_DESTROY(enemy_id);

            event_trigger(EVENT_PLAYER_DAMAGE, nullptr);
        }
    }
}

static void on_player_damage(void* data [[maybe_unused]]) {
    auto state = ECS_GET(g_state->entities.player, PlayerStateComponent);
    auto pos   = ECS_GET(g_state->entities.player, PositionComponent);

    // Decrement lives
    g_state->lives--;

    // Create explosion at player position
    make_explosion(pos->x, pos->y);
    cf_play_sound(g_state->audio.explosion, cf_sound_params_defaults());

    // Mark player as dead
    state->is_alive      = false;
    state->is_invincible = false;

    // Set respawn delay if player has lives remaining
    if (g_state->lives > 0) {
        state->respawn_delay = 2.0f;  // 2 second respawn delay
    }
}

void init_events(void) {
    event_clear_listeners();
    event_register(EVENT_COLLISION, on_collision_explosion);
    event_register(EVENT_COLLISION, on_collision_destroy_bullet_or_enemy);
    event_register(EVENT_COLLISION, on_collision_player_damage);
    event_register(EVENT_PLAYER_DAMAGE, on_player_damage);
}
