#include "audio.h"

#include <cute_audio.h>
#include <cute_result.h>
#include <string.h>

#include "../../engine/log.h"

static inline bool has_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) { return false; }

    return strcmp(dot + 1, extension) == 0;
}

bool load_audio(CF_Audio* audio, const char* path) {
    if (has_extension(path, "wav")) {
        *audio = cf_audio_load_wav(path);

        return true;
    } else if (has_extension(path, "ogg")) {
        *audio = cf_audio_load_ogg(path);

        return true;
    }

    APP_ERROR("Unsupported audio format for %s", path);
    return false;
}
