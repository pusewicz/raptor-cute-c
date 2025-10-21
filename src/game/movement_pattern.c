#include "movement_pattern.h"

#include <cute_math.h>
#include <cute_time.h>
#include <math.h>

// Helper function to normalize a vector safely
static CF_V2 safe_normalize(CF_V2 v) {
    float len = cf_len(v);
    if (len < 0.0001f) {
        return cf_v2(0, 0);
    }
    return cf_norm(v);
}

// Linear pattern: straight movement in a direction
MovementPatternConfig make_linear_pattern(CF_V2 direction, float speed) {
    MovementPatternConfig config = {
        .type              = MOVEMENT_PATTERN_LINEAR,
        .speed             = speed,
        .time              = 0.0f,
        .linear.direction  = safe_normalize(direction),
    };
    return config;
}

// Wave pattern: sine/cosine wave movement
MovementPatternConfig make_wave_pattern(CF_V2 direction, float speed, float amplitude, float frequency, bool horizontal) {
    MovementPatternConfig config = {
        .type               = MOVEMENT_PATTERN_WAVE,
        .speed              = speed,
        .time               = 0.0f,
        .wave.direction     = safe_normalize(direction),
        .wave.amplitude     = amplitude,
        .wave.frequency     = frequency,
        .wave.horizontal    = horizontal,
    };
    return config;
}

// Circular pattern: orbit around a point
MovementPatternConfig make_circular_pattern(CF_V2 center, float radius, float angular_speed) {
    MovementPatternConfig config = {
        .type                    = MOVEMENT_PATTERN_CIRCULAR,
        .speed                   = angular_speed,
        .time                    = 0.0f,
        .circular.center         = center,
        .circular.radius         = radius,
        .circular.angular_speed  = angular_speed,
        .circular.start_angle    = 0.0f,
    };
    return config;
}

// Zigzag pattern: sharp directional changes
MovementPatternConfig make_zigzag_pattern(CF_V2 direction, float speed, float interval, float angle_change) {
    MovementPatternConfig config = {
        .type                   = MOVEMENT_PATTERN_ZIGZAG,
        .speed                  = speed,
        .time                   = 0.0f,
        .zigzag.direction       = safe_normalize(direction),
        .zigzag.interval        = interval,
        .zigzag.angle_change    = angle_change,
        .zigzag.zigzag_count    = 0,
    };
    return config;
}

// Homing pattern: track player
MovementPatternConfig make_homing_pattern(float speed, float turn_speed, bool use_prediction) {
    MovementPatternConfig config = {
        .type                   = MOVEMENT_PATTERN_HOMING,
        .speed                  = speed,
        .time                   = 0.0f,
        .homing.turn_speed      = turn_speed,
        .homing.use_prediction  = use_prediction,
    };
    return config;
}

// Swooping pattern: dive toward player
MovementPatternConfig make_swooping_pattern(CF_V2 entry_pos, CF_V2 target_pos, float swoop_speed) {
    MovementPatternConfig config = {
        .type                      = MOVEMENT_PATTERN_SWOOPING,
        .speed                     = swoop_speed,
        .time                      = 0.0f,
        .swooping.entry_position   = entry_pos,
        .swooping.target_position  = target_pos,
        .swooping.swoop_speed      = swoop_speed,
        .swooping.state            = 0,
    };
    return config;
}

// Stop and shoot pattern: move, stop, move
MovementPatternConfig make_stop_shoot_pattern(CF_V2 direction, float speed, float move_duration, float stop_duration) {
    MovementPatternConfig config = {
        .type                       = MOVEMENT_PATTERN_STOP_SHOOT,
        .speed                      = speed,
        .time                       = 0.0f,
        .stop_shoot.direction       = safe_normalize(direction),
        .stop_shoot.move_duration   = move_duration,
        .stop_shoot.stop_duration   = stop_duration,
        .stop_shoot.is_stopped      = false,
    };
    return config;
}

// Update movement pattern and calculate new velocity
void update_movement_pattern(MovementPatternConfig* pattern, CF_V2* position, CF_V2* velocity, CF_V2 player_position) {
    pattern->time += CF_DELTA_TIME;

    switch (pattern->type) {
        case MOVEMENT_PATTERN_LINEAR: {
            // Simple linear movement
            velocity->x = pattern->linear.direction.x * pattern->speed;
            velocity->y = pattern->linear.direction.y * pattern->speed;
            break;
        }

        case MOVEMENT_PATTERN_WAVE: {
            // Wave movement: base direction + perpendicular oscillation
            float wave_offset = sinf(pattern->time * pattern->wave.frequency) * pattern->wave.amplitude;

            if (pattern->wave.horizontal) {
                // Horizontal wave: oscillate left-right while moving in base direction
                velocity->x = pattern->wave.direction.x * pattern->speed + wave_offset;
                velocity->y = pattern->wave.direction.y * pattern->speed;
            } else {
                // Vertical wave: oscillate up-down while moving in base direction
                velocity->x = pattern->wave.direction.x * pattern->speed;
                velocity->y = pattern->wave.direction.y * pattern->speed + wave_offset;
            }
            break;
        }

        case MOVEMENT_PATTERN_CIRCULAR: {
            // Circular/orbital movement
            float angle  = pattern->circular.start_angle + pattern->time * pattern->circular.angular_speed;
            float new_x  = pattern->circular.center.x + cosf(angle) * pattern->circular.radius;
            float new_y  = pattern->circular.center.y + sinf(angle) * pattern->circular.radius;

            // Calculate velocity as the difference from current position
            velocity->x  = (new_x - position->x) / CF_DELTA_TIME;
            velocity->y  = (new_y - position->y) / CF_DELTA_TIME;
            break;
        }

        case MOVEMENT_PATTERN_ZIGZAG: {
            // Zigzag: change direction at intervals
            int   zigzag_num = (int)(pattern->time / pattern->zigzag.interval);
            if (zigzag_num > pattern->zigzag.zigzag_count) {
                pattern->zigzag.zigzag_count = zigzag_num;

                // Rotate direction by angle_change
                float angle_rad                = pattern->zigzag.angle_change * (CF_PI / 180.0f);
                float cos_angle                = cosf(angle_rad);
                float sin_angle                = sinf(angle_rad);
                float new_x                    = pattern->zigzag.direction.x * cos_angle - pattern->zigzag.direction.y * sin_angle;
                float new_y                    = pattern->zigzag.direction.x * sin_angle + pattern->zigzag.direction.y * cos_angle;

                pattern->zigzag.direction      = safe_normalize(cf_v2(new_x, new_y));
            }

            velocity->x = pattern->zigzag.direction.x * pattern->speed;
            velocity->y = pattern->zigzag.direction.y * pattern->speed;
            break;
        }

        case MOVEMENT_PATTERN_HOMING: {
            // Homing: turn toward player
            CF_V2 to_player = cf_sub_v2(player_position, *position);
            CF_V2 desired   = safe_normalize(to_player);

            // Smooth turn using lerp
            CF_V2 current   = safe_normalize(*velocity);
            float t         = pattern->homing.turn_speed * CF_DELTA_TIME;
            CF_V2 new_dir   = cf_norm(cf_v2(
                current.x + (desired.x - current.x) * t,
                current.y + (desired.y - current.y) * t
            ));

            velocity->x     = new_dir.x * pattern->speed;
            velocity->y     = new_dir.y * pattern->speed;
            break;
        }

        case MOVEMENT_PATTERN_SWOOPING: {
            // Swooping: enter, dive toward target, exit
            if (pattern->swooping.state == 0) {
                // Entering phase - move to starting position
                velocity->x = 0;
                velocity->y = -pattern->speed * 0.5f;  // Slower entry

                // Transition to swooping after some time
                if (pattern->time > 1.0f) {
                    pattern->swooping.state = 1;
                }
            } else if (pattern->swooping.state == 1) {
                // Swooping phase - dive toward target
                CF_V2 to_target = cf_sub_v2(pattern->swooping.target_position, *position);
                CF_V2 dir       = safe_normalize(to_target);
                velocity->x     = dir.x * pattern->swooping.swoop_speed;
                velocity->y     = dir.y * pattern->swooping.swoop_speed;

                // Check if we've passed the target or gotten close enough
                if (position->y < pattern->swooping.target_position.y - 20.0f || cf_len(to_target) < 10.0f) {
                    pattern->swooping.state = 2;
                }
            } else {
                // Exiting phase - continue in same direction
                velocity->y = -pattern->speed * 1.5f;  // Faster exit
            }
            break;
        }

        case MOVEMENT_PATTERN_STOP_SHOOT: {
            // Stop and shoot: alternate between moving and stopping
            float cycle_time = pattern->stop_shoot.move_duration + pattern->stop_shoot.stop_duration;
            float phase      = fmodf(pattern->time, cycle_time);

            if (phase < pattern->stop_shoot.move_duration) {
                // Moving phase
                pattern->stop_shoot.is_stopped = false;
                velocity->x                    = pattern->stop_shoot.direction.x * pattern->speed;
                velocity->y                    = pattern->stop_shoot.direction.y * pattern->speed;
            } else {
                // Stopped phase
                pattern->stop_shoot.is_stopped = true;
                velocity->x                    = 0;
                velocity->y                    = 0;
            }
            break;
        }

        case MOVEMENT_PATTERN_FORMATION:
            // Formation movement is handled at the group level
            // Individual enemies in formation just follow their offset from formation center
            // This is a placeholder - actual formation logic would be more complex
            velocity->x = 0;
            velocity->y = -pattern->speed;
            break;
    }
}
