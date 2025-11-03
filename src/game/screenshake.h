#pragma once

#include <cute_math.h>

// Maximum shake magnitude to prevent excessive shaking
constexpr float SCREENSHAKE_MAX_MAGNITUDE          = 10.0f;

// Screenshake oscillation frequencies
constexpr float SCREENSHAKE_FREQ_TIME_SCALE        = 30.0f;  // Time multiplier for shake speed
constexpr float SCREENSHAKE_FREQ_OFFSET_X1         = 1.3f;   // Primary X offset frequency
constexpr float SCREENSHAKE_FREQ_OFFSET_Y1         = 1.7f;   // Primary Y offset frequency
constexpr float SCREENSHAKE_FREQ_OFFSET_X2         = 3.1f;   // Secondary X offset frequency (jitter)
constexpr float SCREENSHAKE_FREQ_OFFSET_Y2         = 2.9f;   // Secondary Y offset frequency (jitter)
constexpr float SCREENSHAKE_FREQ_ROTATION_1        = 2.3f;   // Primary rotation frequency
constexpr float SCREENSHAKE_FREQ_ROTATION_2        = 1.9f;   // Secondary rotation frequency

// Screenshake scaling factors
constexpr float SCREENSHAKE_OFFSET_JITTER_SCALE    = 0.5f;    // Scale for high-frequency jitter
constexpr float SCREENSHAKE_ROTATION_SCALE         = 0.005f;  // Max rotation intensity (~2-3 degrees at max)
constexpr float SCREENSHAKE_ROTATION_COUNTER_SCALE = 0.6f;    // Scale for counter-oscillation

// Screenshake state structure
typedef struct ScreenShake {
    float magnitude;   // Current shake intensity
    float decay_rate;  // How fast the shake fades (higher = faster)
    float time;        // Internal timer for variation
    CF_V2 offset;      // Current offset to apply to canvas
    float rotation;    // Current rotation angle in radians
} ScreenShake;

// Initialize screenshake system
void screenshake_init(ScreenShake* shake, float decay_rate);

// Add shake intensity (call this when explosion happens)
void screenshake_add(ScreenShake* shake, float amount);

// Update shake (call every frame with delta time)
void screenshake_update(ScreenShake* shake);

// Get the current offset to apply to canvas position
CF_V2 screenshake_get_offset(const ScreenShake* shake);

// Get the current rotation angle in radians
float screenshake_get_rotation(const ScreenShake* shake);
