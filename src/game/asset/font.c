#include "font.h"

#include <cute_draw.h>
#include <cute_result.h>

#include "../../engine/log.h"

bool load_font(const char* path, const char* font_name) {
    CF_Result result = cf_make_font(path, font_name);

    if (cf_is_error(result)) {
        APP_ERROR("Could not load font: %s", result.details != nullptr ? result.details : "No details");
        return false;
    }

    return true;
}
