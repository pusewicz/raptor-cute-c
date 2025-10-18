#include "coroutine.h"

#include <cute_alloc.h>
#include <cute_coroutine.h>
#include <cute_defines.h>
#include <cute_time.h>

#include "../engine/game_state.h"
#include "enemy.h"

static void wait_for(float seconds) {
    auto   coro     = cf_coroutine_currently_running();
    double end_time = CF_SECONDS + (double)seconds;

    while (CF_SECONDS < end_time) { cf_coroutine_yield(coro); }
}

static void enemy_spawner(CF_Coroutine co [[maybe_unused]]) {
    while (1) {
        float canvas_top = g_state->canvas_size.y / 2.0f;

        spawn_enemy(make_random_enemy(0, canvas_top));
        wait_for(2.0f);

        spawn_enemy(make_random_enemy(-10, canvas_top));
        spawn_enemy(make_random_enemy(10, canvas_top));
        wait_for(1.0f);
    }
}

void init_coroutines(void) { g_state->coroutines.spawner = cf_make_coroutine(enemy_spawner, CF_MB, nullptr); }
void cleanup_coroutines(void) { cf_destroy_coroutine(g_state->coroutines.spawner); }
void update_coroutine(void) {
    if (cf_coroutine_state(g_state->coroutines.spawner) != CF_COROUTINE_STATE_DEAD) {
        cf_coroutine_resume(g_state->coroutines.spawner);
    }
}
