#include "event.h"

#include <cute_c_runtime.h>
#include <cute_math.h>

#include "../engine/game_state.h"
#include "../engine/log.h"
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
    APP_DEBUG(
        "Registering event listener for event type %s",
        event_type_to_string(type)
    );
    int count                    = event_listener_counts[type];
    event_listeners[type][count] = callback;
    event_listener_counts[type]++;
}

void event_trigger(EventType type, void* data) {
    for (int i = 0; i < event_listener_counts[type]; i++) {
        event_listeners[type][i](data);
    }
}

void event_clear_listeners(void) {
    APP_DEBUG("Clearing event listeners...");
    for (int i = 0; i < EVENT_COUNT; i++) {
        for (int j = 0; j < event_listener_counts[i]; j++) {
            event_listeners[i][j] = nullptr;
        }
        event_listener_counts[i] = 0;
    }
}

static void on_collision_destroy_bullet_or_enemy(void* data) {
    auto e = (CollisionEvent*)data;
    {
        auto tag_a = ECS_GET(e->entity_a, TagComponent);
        auto tag_b = ECS_GET(e->entity_b, TagComponent);

        if ((*tag_a == TAG_BULLET && *tag_b == TAG_ENEMY) ||
            (*tag_a == TAG_ENEMY && *tag_b == TAG_BULLET)) {
            ECS_QUEUE_DESTROY(e->entity_a);
            ECS_QUEUE_DESTROY(e->entity_b);
        }
    }
}

static void on_collision_explosion(void* data) {
    auto e = (CollisionEvent*)data;
    {
        auto tag_a = ECS_GET(e->entity_a, TagComponent);
        auto tag_b = ECS_GET(e->entity_b, TagComponent);

        if ((*tag_a == TAG_BULLET && *tag_b == TAG_ENEMY) ||
            (*tag_a == TAG_ENEMY && *tag_b == TAG_BULLET)) {
            auto pos_a         = ECS_GET(e->entity_a, PositionComponent);
            auto pos_b         = ECS_GET(e->entity_b, PositionComponent);
            auto explosion_pos = (*tag_a == TAG_BULLET) ? pos_b : pos_a;

            make_explosion(explosion_pos->x, explosion_pos->y);
        }
    }
}

void init_events(void) {
    event_clear_listeners();
    event_register(EVENT_COLLISION, on_collision_explosion);
    event_register(EVENT_COLLISION, on_collision_destroy_bullet_or_enemy);
}
