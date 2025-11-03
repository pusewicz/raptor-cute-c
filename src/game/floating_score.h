#pragma once

#include <cute_math.h>
#include <stdbool.h>
#include <stddef.h>

typedef struct FloatingScore {
    CF_V2 position;
    CF_V2 velocity;
    int   score;
    float lifetime;
    float alpha;
    bool  is_alive;
} FloatingScore;

FloatingScore make_floating_score(CF_V2 position, int score);
void          spawn_floating_score(FloatingScore floating_score);
void          update_floating_scores(void);
void          render_floating_scores(void);
void          cleanup_floating_scores(void);
