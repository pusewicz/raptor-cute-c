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

bool load_sprite(CF_Sprite* sprite, const char* path) {
    if (has_extension(path, "png")) {
        CF_Result result = {0};
        *sprite          = cf_make_easy_sprite_from_png(path, &result);

        if (cf_is_error(result)) {
            APP_ERROR("Could not load sprite: %s", result.details != NULL ? result.details : "No details");
            return false;
        }

        return true;
    } else if (has_extension(path, "ase") || has_extension(path, "aseprite")) {
        *sprite = cf_make_sprite(path);
        return true;
    }

    APP_ERROR("Unsupported sprite format for %s", path);
    return false;
}
