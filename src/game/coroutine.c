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

static void spawn_single_enemy(CF_V2 position, EnemyType type, float shoot_chance) {
    auto enemy = make_enemy_of_type(position, type);
    set_enemy_shoot_chance(&enemy, shoot_chance);
    spawn_enemy(enemy);
}

static void enemy_spawner(CF_Coroutine co [[maybe_unused]]) {
    while (1) {
        float canvas_top = g_state->canvas_size.y / 2.0f;
        CF_V2 spawn_pos  = cf_v2(0, canvas_top);
        int   wave       = g_state->wave.current_wave;
        float shoot_chance;
        CF_V2 left_pos, right_pos;

        // Wait for wave announcement to finish
        while (g_state->wave.is_announcing) { cf_coroutine_yield(co); }

        switch (wave) {
            case 0:
                // Wave 0: 3 single ALAN enemies, no shooting
                shoot_chance = 0.0f;
                spawn_single_enemy(cf_v2(-20, canvas_top), ENEMY_TYPE_ALAN, shoot_chance);
                wait_for(1.0f);
                spawn_single_enemy(cf_v2(0, canvas_top), ENEMY_TYPE_ALAN, shoot_chance);
                wait_for(1.0f);
                spawn_single_enemy(cf_v2(20, canvas_top), ENEMY_TYPE_ALAN, shoot_chance);
                wait_for(2.0f);
                break;

            case 1:
                // Wave 1: 5 single ALAN enemies, still no shooting
                shoot_chance = 0.0f;
                for (int i = 0; i < 5; i++) {
                    float x_pos = cf_rnd_range_float(&g_state->rnd, -60.0f, 60.0f);
                    spawn_single_enemy(cf_v2(x_pos, canvas_top), ENEMY_TYPE_ALAN, shoot_chance);
                    wait_for(0.8f);
                }
                wait_for(1.5f);
                break;

            case 2:
                // Wave 2: BON_BON line formation, 10% shoot chance
                shoot_chance = 0.1f;
                formation_spawn_with_shoot_chance(
                    &FORMATION_LINE_HORIZONTAL, spawn_pos, ENEMY_TYPE_BON_BON, shoot_chance
                );
                wait_for(3.0f);
                break;

            case 3:
                // Wave 3: Mixed enemy types, 20% shoot chance
                shoot_chance = 0.2f;
                left_pos     = cf_v2(-30, canvas_top);
                right_pos    = cf_v2(30, canvas_top);
                formation_spawn_with_shoot_chance(&FORMATION_LINE_VERTICAL, left_pos, ENEMY_TYPE_ALAN, shoot_chance);
                wait_for(1.0f);
                formation_spawn_with_shoot_chance(
                    &FORMATION_LINE_VERTICAL, right_pos, ENEMY_TYPE_BON_BON, shoot_chance
                );
                wait_for(2.5f);
                break;

            case 4:
                // Wave 4: LIPS diamond formation, 30% shoot chance
                shoot_chance = 0.3f;
                formation_spawn_with_shoot_chance(&FORMATION_DIAMOND, spawn_pos, ENEMY_TYPE_LIPS, shoot_chance);
                wait_for(4.0f);
                break;

            case 5:
                // Wave 5: Wave formation with BON_BON, 40% shoot chance
                shoot_chance = 0.4f;
                formation_spawn_with_shoot_chance(&FORMATION_WAVE, spawn_pos, ENEMY_TYPE_BON_BON, shoot_chance);
                wait_for(3.5f);
                break;

            case 6:
                // Wave 6: Arrow formation with LIPS, 50% shoot chance
                shoot_chance = 0.5f;
                formation_spawn_with_shoot_chance(&FORMATION_ARROW_DOWN, spawn_pos, ENEMY_TYPE_LIPS, shoot_chance);
                wait_for(4.0f);
                break;

            default:
                // Wave 7+: Multiple formations, shoot chance increases with wave (capped at 70%)
                shoot_chance = cf_min(0.7f, 0.3f + (wave - 6) * 0.05f);

                // Spawn multiple formations
                formation_spawn_with_shoot_chance(&FORMATION_V_SHAPE, spawn_pos, ENEMY_TYPE_BON_BON, shoot_chance);
                wait_for(2.0f);
                formation_spawn_with_shoot_chance(&FORMATION_DIAMOND, spawn_pos, ENEMY_TYPE_LIPS, shoot_chance);
                wait_for(2.5f);
                formation_spawn_with_shoot_chance(&FORMATION_WAVE, spawn_pos, ENEMY_TYPE_ALAN, shoot_chance);
                wait_for(1.5f);
                break;
        }

        // Wait for all enemies to be cleared before starting next wave
        while (g_state->enemies_count > 0) { cf_coroutine_yield(co); }

        // Start next wave
        g_state->wave.current_wave++;
        g_state->wave.announcement_timer = 0.0f;
        g_state->wave.is_announcing      = true;

        // Small delay before announcing next wave
        wait_for(3.0f);
    }
}

void init_coroutines(void) {
    g_state->coroutines.spawner     = cf_make_coroutine(enemy_spawner, CF_MB, nullptr);
    g_state->coroutines.initialized = true;
}

void cleanup_coroutines(void) {
    if (g_state->coroutines.initialized) {
        cf_destroy_coroutine(g_state->coroutines.spawner);
        g_state->coroutines.initialized = false;
    }
}
void update_coroutine(void) {
    if (cf_coroutine_state(g_state->coroutines.spawner) != CF_COROUTINE_STATE_DEAD) {
        cf_coroutine_resume(g_state->coroutines.spawner);
    }
}
