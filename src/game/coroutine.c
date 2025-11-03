#include "coroutine.h"

#include <cute_alloc.h>
#include <cute_coroutine.h>
#include <cute_defines.h>
#include <cute_math.h>
#include <cute_time.h>

#include "../engine/game_state.h"
#include "enemy.h"
#include "formation.h"

static void wait_for(float seconds) {
    auto   coro     = cf_coroutine_currently_running();
    double end_time = CF_SECONDS + (double)seconds;

    while (CF_SECONDS < end_time) { cf_coroutine_yield(coro); }
}

static void enemy_spawner(CF_Coroutine co [[maybe_unused]]) {
    while (1) {
        float canvas_top      = g_state->canvas_size.y / 2.0f;

        const CF_V2 spawn_pos = cf_v2(0, canvas_top);
        formation_spawn(&FORMATION_DIAMOND, spawn_pos, ENEMY_TYPE_ALAN);
        wait_for(3.0f);

        formation_spawn(&FORMATION_LINE_HORIZONTAL, spawn_pos, ENEMY_TYPE_BON_BON);
        wait_for(3.0f);

        formation_spawn(&FORMATION_WAVE, spawn_pos, ENEMY_TYPE_LIPS);
        wait_for(3.0f);

        formation_spawn(&FORMATION_ARROW_DOWN, spawn_pos, ENEMY_TYPE_BON_BON);
        wait_for(3.0f);
    }
}

void init_coroutines(void) { g_state->coroutines.spawner = cf_make_coroutine(enemy_spawner, CF_MB, nullptr); }
void cleanup_coroutines(void) { cf_destroy_coroutine(g_state->coroutines.spawner); }
void update_coroutine(void) {
    if (cf_coroutine_state(g_state->coroutines.spawner) != CF_COROUTINE_STATE_DEAD) {
        cf_coroutine_resume(g_state->coroutines.spawner);
    }
}
