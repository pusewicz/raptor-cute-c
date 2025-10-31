#include "audio.h"

#include <cute_audio.h>
#include <cute_c_runtime.h>
#include <cute_result.h>
#include <string.h>

#include "../../engine/game_state.h"
#include "../../engine/log.h"

static const char* const s_audio_files[AUDIO_COUNT] = {
    [MUSIC_BACKGROUND] = "assets/music.ogg",
    [SOUND_REVEAL]     = "assets/reveal.ogg",
    [SOUND_GAME_OVER]  = "assets/game-over.ogg",
    [SOUND_DEATH]      = "assets/death.ogg",
    [SOUND_LASER]      = "assets/laser-shoot.ogg",
    [SOUND_EXPLOSION]  = "assets/explosion.ogg",
    [SOUND_HIT]        = "assets/hit-hurt.ogg",
};

static inline bool has_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) { return false; }

    return strcmp(dot + 1, extension) == 0;
}

CF_Audio load_audio(const char* path) {
    CF_ASSERT(path);

    CF_Audio audio = {0};

    if (has_extension(path, "wav")) {
        audio = cf_audio_load_wav(path);

        return audio;
    } else if (has_extension(path, "ogg")) {
        audio = cf_audio_load_ogg(path);

        return audio;
    }

    APP_ERROR("Unsupported audio format for %s", path);
    return audio;
}

void load_audios() {
    for (size_t i = 0; i < AUDIO_COUNT; ++i) { g_state->audio_assets[i] = load_audio(s_audio_files[i]); }
}

CF_Audio get_audio(const Audio audio) { return g_state->audio_assets[audio]; }

void play_sound(const Audio audio) { cf_play_sound(get_audio(audio), cf_sound_params_defaults()); }
void play_music(const Audio audio) { cf_music_play(get_audio(audio), 0.5f); }
