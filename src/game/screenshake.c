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

    // Cap the maximum shake to prevent excessive shaking
    if (shake->magnitude > SCREENSHAKE_MAX_MAGNITUDE) { shake->magnitude = SCREENSHAKE_MAX_MAGNITUDE; }
}

void screenshake_update(ScreenShake* shake) {
    if (shake->magnitude <= 0.0f) {
        shake->offset   = cf_v2(0.0f, 0.0f);
        shake->rotation = 0.0f;
        return;
    }

    // Update time for variation
    shake->time += CF_DELTA_TIME * SCREENSHAKE_FREQ_TIME_SCALE;

    // Use Perlin-noise-like variation with sine waves at different frequencies
    // This creates smooth but chaotic motion
    float x_offset = CF_SINF(shake->time * SCREENSHAKE_FREQ_OFFSET_X1) * shake->magnitude;
    float y_offset = CF_COSF(shake->time * SCREENSHAKE_FREQ_OFFSET_Y1) * shake->magnitude;

    // Add higher frequency component for more "jittery" feel
    x_offset += CF_SINF(shake->time * SCREENSHAKE_FREQ_OFFSET_X2) * shake->magnitude * SCREENSHAKE_OFFSET_JITTER_SCALE;
    y_offset += CF_COSF(shake->time * SCREENSHAKE_FREQ_OFFSET_Y2) * shake->magnitude * SCREENSHAKE_OFFSET_JITTER_SCALE;

    shake->offset = cf_v2(x_offset, y_offset);

    // Calculate rotation - use different frequency for independence from position
    shake->rotation =
        CF_SINF(shake->time * SCREENSHAKE_FREQ_ROTATION_1) * shake->magnitude * SCREENSHAKE_ROTATION_SCALE;

    // Add a counter-oscillation for more dynamic rotation
    shake->rotation += CF_COSF(shake->time * SCREENSHAKE_FREQ_ROTATION_2) * shake->magnitude *
                       SCREENSHAKE_ROTATION_SCALE * SCREENSHAKE_ROTATION_COUNTER_SCALE;

    // Decay the magnitude over time
    shake->magnitude -= shake->decay_rate * CF_DELTA_TIME;
    if (shake->magnitude < 0.0f) { shake->magnitude = 0.0f; }
}

CF_V2 screenshake_get_offset(const ScreenShake* shake) { return shake->offset; }

float screenshake_get_rotation(const ScreenShake* shake) { return shake->rotation; }
