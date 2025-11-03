#include "screenshake.h"

#include <cute_math.h>
#include <cute_time.h>

void screenshake_init(ScreenShake* shake, float decay_rate) {
    shake->magnitude  = 0.0f;
    shake->decay_rate = decay_rate;
    shake->time       = 0.0f;
    shake->offset     = cf_v2(0.0f, 0.0f);
    shake->rotation   = 0.0f;
}

void screenshake_add(ScreenShake* shake, float amount) {
    // Add to existing magnitude (allows multiple explosions to stack)
    shake->magnitude += amount;

    // Optional: Cap the maximum shake to prevent excessive shaking
    const float MAX_SHAKE = 10.0f;
    if (shake->magnitude > MAX_SHAKE) { shake->magnitude = MAX_SHAKE; }
}

void screenshake_update(ScreenShake* shake) {
    if (shake->magnitude <= 0.0f) {
        shake->offset   = cf_v2(0.0f, 0.0f);
        shake->rotation = 0.0f;
        return;
    }

    // Update time for variation
    shake->time += CF_DELTA_TIME * 30.0f;  // Multiply by frequency for faster shake

    // Use Perlin-noise-like variation with sine waves at different frequencies
    // This creates smooth but chaotic motion
    float x_offset = CF_SINF(shake->time * 1.3f) * shake->magnitude;
    float y_offset = CF_COSF(shake->time * 1.7f) * shake->magnitude;

    // Add higher frequency component for more "jittery" feel
    x_offset += CF_SINF(shake->time * 3.1f) * shake->magnitude * 0.5f;
    y_offset += CF_COSF(shake->time * 2.9f) * shake->magnitude * 0.5f;

    shake->offset        = cf_v2(x_offset, y_offset);

    // Calculate rotation - use different frequency for independence from position
    // Scale rotation by magnitude but keep it subtle (convert to degrees conceptually)
    // Max rotation at max magnitude should be around 2-3 degrees (0.035 - 0.052 radians)
    float rotation_scale = 0.005f;  // Adjust this to control max rotation intensity
    shake->rotation      = CF_SINF(shake->time * 2.3f) * shake->magnitude * rotation_scale;

    // Add a counter-oscillation for more dynamic rotation
    shake->rotation += CF_COSF(shake->time * 1.9f) * shake->magnitude * rotation_scale * 0.6f;

    // Decay the magnitude over time
    shake->magnitude -= shake->decay_rate * CF_DELTA_TIME;
    if (shake->magnitude < 0.0f) { shake->magnitude = 0.0f; }
}

CF_V2 screenshake_get_offset(const ScreenShake* shake) { return shake->offset; }

float screenshake_get_rotation(const ScreenShake* shake) { return shake->rotation; }
