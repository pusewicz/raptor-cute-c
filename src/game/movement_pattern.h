#pragma once

#include <cute_math.h>

// Movement pattern types for enemies
typedef enum MovementPattern {
    MOVEMENT_PATTERN_LINEAR,       // Straight down/across at various speeds
    MOVEMENT_PATTERN_WAVE,         // Sine/cosine wave movements
    MOVEMENT_PATTERN_CIRCULAR,     // Circular/orbital movements
    MOVEMENT_PATTERN_FORMATION,    // Formation flying (handled at group level)
    MOVEMENT_PATTERN_ZIGZAG,       // Sharp directional changes
    MOVEMENT_PATTERN_HOMING,       // Track player position
    MOVEMENT_PATTERN_SWOOPING,     // Dive toward player then exit
    MOVEMENT_PATTERN_STOP_SHOOT,   // Enter, pause to fire, reposition
} MovementPattern;

// Movement pattern configuration
typedef struct MovementPatternConfig {
    MovementPattern type;

    // Common parameters
    float speed;              // Base speed of movement
    float time;              // Time accumulator for pattern calculations

    // Pattern-specific parameters
    union {
        // Linear pattern
        struct {
            CF_V2 direction;  // Movement direction (normalized)
        } linear;

        // Wave pattern
        struct {
            CF_V2  direction;      // Base movement direction
            float  amplitude;      // Wave amplitude
            float  frequency;      // Wave frequency
            bool   horizontal;     // true = horizontal wave, false = vertical wave
        } wave;

        // Circular pattern
        struct {
            CF_V2  center;         // Center point for orbit
            float  radius;         // Orbit radius
            float  angular_speed;  // Speed of rotation (radians/sec)
            float  start_angle;    // Starting angle
        } circular;

        // Zigzag pattern
        struct {
            CF_V2  direction;      // Base movement direction
            float  interval;       // Time between direction changes
            float  angle_change;   // Angle to change by (degrees)
            int    zigzag_count;   // Number of zigzags performed
        } zigzag;

        // Homing pattern
        struct {
            float  turn_speed;     // How fast to turn toward player
            bool   use_prediction; // Use predicted player position
        } homing;

        // Swooping pattern
        struct {
            CF_V2  entry_position; // Starting position
            CF_V2  target_position;// Where to swoop toward
            float  swoop_speed;    // Speed during swoop
            int    state;          // 0=entering, 1=swooping, 2=exiting
        } swooping;

        // Stop and shoot pattern
        struct {
            CF_V2  direction;      // Movement direction
            float  move_duration;  // How long to move
            float  stop_duration;  // How long to stop
            bool   is_stopped;     // Current state
        } stop_shoot;
    };
} MovementPatternConfig;

// Initialize movement pattern configurations
MovementPatternConfig make_linear_pattern(CF_V2 direction, float speed);
MovementPatternConfig make_wave_pattern(CF_V2 direction, float speed, float amplitude, float frequency, bool horizontal);
MovementPatternConfig make_circular_pattern(CF_V2 center, float radius, float angular_speed);
MovementPatternConfig make_zigzag_pattern(CF_V2 direction, float speed, float interval, float angle_change);
MovementPatternConfig make_homing_pattern(float speed, float turn_speed, bool use_prediction);
MovementPatternConfig make_swooping_pattern(CF_V2 entry_pos, CF_V2 target_pos, float swoop_speed);
MovementPatternConfig make_stop_shoot_pattern(CF_V2 direction, float speed, float move_duration, float stop_duration);

// Update movement pattern and modify velocity
void update_movement_pattern(MovementPatternConfig* pattern, CF_V2* position, CF_V2* velocity, CF_V2 player_position);
