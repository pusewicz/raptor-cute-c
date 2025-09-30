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
        APP_DEBUG("Spawning enemy");
        make_enemy(100, 50);
        APP_DEBUG("Sleeping for 2 seconds");
        co_sleep(co, 2.0f);

        APP_DEBUG("Spawning 2 enemies");
        make_enemy(200, 50);
        make_enemy(300, 50);
        APP_DEBUG("Sleeping for 1 second");
        co_sleep(co, 1.0f);
    }
}

void init_coroutines(void) { g_state->coroutines.spawner = cf_make_coroutine(enemy_spawner, CF_MB, nullptr); }
void cleanup_coroutines(void) { cf_destroy_coroutine(g_state->coroutines.spawner); }
