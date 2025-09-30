#include "coroutine.h"

#include <cute_alloc.h>
#include <cute_coroutine.h>
#include <cute_defines.h>
#include <cute_time.h>

#include "../engine/game_state.h"
#include "../engine/log.h"
#include "factories.h"

void co_sleep(CF_Coroutine co, float seconds) {
    float elapsed = 0.0f;
    while (elapsed < seconds) {
        cf_coroutine_yield(co);
        elapsed += CF_DELTA_TIME;
    }
}

void enemy_spawner(CF_Coroutine co) {
    while (1) {
        float canvas_top = g_state->canvas_size.y / 2.0f;

        make_enemy(0, canvas_top);
        co_sleep(co, 2.0f);

        make_enemy(-10, canvas_top);
        make_enemy(10, canvas_top);
        co_sleep(co, 1.0f);
    }
}

void init_coroutines(void) { g_state->coroutines.spawner = cf_make_coroutine(enemy_spawner, CF_MB, nullptr); }
void cleanup_coroutines(void) { cf_destroy_coroutine(g_state->coroutines.spawner); }
