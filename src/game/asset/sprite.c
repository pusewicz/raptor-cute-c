#include "sprite.h"

#include <cute_result.h>
#include <cute_sprite.h>
#include <string.h>

#include "../../engine/log.h"

static inline bool has_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) { return false; }

    return strcmp(dot + 1, extension) == 0;
}

bool load_sprite(void* asset, const char* path) {
    CF_Sprite* sprite = asset;

    if (has_extension(path, "png")) {
        CF_Result result = {0};
        *sprite          = cf_make_easy_sprite_from_png(path, &result);
        if (result.code != CF_RESULT_SUCCESS) {
            APP_ERROR("Could not load sprite: %s", result.details != NULL ? result.details : "No details");
            return false;
        } else {
            return true;
        }
    } else if (has_extension(path, "ase") || has_extension(path, "aseprite")) {
        *sprite = cf_make_sprite(path);
        return true;
    }

    APP_ERROR("Unsupported sprite format for %s", path);
    return false;
}
