#include "floating_score.h"

#include <cute_c_runtime.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_time.h>
#include <stddef.h>
#include <stdio.h>

#include "../engine/cute_macros.h"
#include "../engine/game_state.h"

constexpr float FLOATING_SCORE_SPEED    = 0.3f;
constexpr float FLOATING_SCORE_LIFETIME = 1.0f;

FloatingScore make_floating_score(float x, float y, int score) {
    return (FloatingScore){
        .position = cf_v2(x, y),
        .velocity = cf_v2(0, FLOATING_SCORE_SPEED),
        .score    = score,
        .lifetime = FLOATING_SCORE_LIFETIME,
        .alpha    = 1.0f,
        .is_alive = true,
    };
}

void spawn_floating_score(FloatingScore floating_score) {
    CF_ASSERT(g_state->floating_scores);
    CF_ASSERT(g_state->floating_scores_count < g_state->floating_scores_capacity);
    g_state->floating_scores[g_state->floating_scores_count++] = floating_score;
}

void update_floating_scores(void) {
    for (size_t i = 0; i < g_state->floating_scores_count; i++) {
        auto score = &g_state->floating_scores[i];
        if (!score->is_alive) { continue; }

        // Move upward
        score->position.y += score->velocity.y;

        // Update lifetime
        score->lifetime -= CF_DELTA_TIME;

        // Fade out
        score->alpha = score->lifetime / FLOATING_SCORE_LIFETIME;

        // Mark as dead when lifetime expires
        if (score->lifetime <= 0.0f) { score->is_alive = false; }
    }
}

void render_floating_scores(void) {
    for (size_t i = 0; i < g_state->floating_scores_count; i++) {
        auto score = &g_state->floating_scores[i];
        if (!score->is_alive) { continue; }

        char score_text[16];
        snprintf(score_text, sizeof(score_text), "%d", score->score);

        cf_draw() {
            cf_font("TinyAndChunky") {
                cf_push_font_size(7);
                float text_width = cf_text_width(score_text, -1);

                cf_draw_layer(Z_UI) {
                    // Draw with alpha for fade effect
                    cf_draw_color(cf_make_color_rgba(255, 255, 255, (int)(score->alpha * 255))) {
                        cf_draw_text(score_text, cf_v2(score->position.x - text_width / 2.0f, score->position.y), -1);
                    }
                }
            }
        }
    }
}

void cleanup_floating_scores(void) {
    int write_idx = 0;
    for (size_t i = 0; i < g_state->floating_scores_count; i++) {
        if (g_state->floating_scores[i].is_alive) {
            g_state->floating_scores[write_idx++] = g_state->floating_scores[i];
        }
    }
    g_state->floating_scores_count = write_idx;
}
