#pragma once

#include <cute_math.h>

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
