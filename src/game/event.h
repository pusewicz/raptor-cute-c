#pragma once

#include <pico_ecs.h>

#define EVENT_LIST            \
    X(EVENT_COLLISION, 0)     \
    X(EVENT_ENEMY_DEATH, 1)   \
    X(EVENT_PLAYER_DAMAGE, 2)

typedef enum {
#define X(name, value) name = value,
    EVENT_LIST
#undef X
        EVENT_COUNT
} EventType;

typedef void (*EventCallback)(void* data);

/*
 * Event Data Structures
 */

typedef struct {
    ecs_id_t entity_a;
    ecs_id_t entity_b;
} CollisionEvent;

constexpr int MAX_EVENT_LISTENERS = 8;

// TODO: Move to game_state.h and initialize in game_init so hot reload works
EventCallback event_listeners[EVENT_COUNT][MAX_EVENT_LISTENERS];
int           event_listener_counts[EVENT_COUNT];

void        event_register(EventType type, EventCallback callback);
void        event_trigger(EventType type, void* data);
void        event_clear_listeners(void);
const char* event_type_to_string(EventType type);
void        init_events(void);
