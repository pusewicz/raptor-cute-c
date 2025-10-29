#include "sprite.h"

#include <cute_c_runtime.h>
#include <cute_draw.h>
#include <cute_math.h>
#include <cute_result.h>
#include <cute_sprite.h>
#include <string.h>

#include "../../engine/cute_macros.h"
#include "../../engine/log.h"
#include "../component.h"

static CF_Sprite s_sprites[SPRITE_COUNT];

static const char* const s_sprite_files[SPRITE_COUNT] = {
    [SPRITE_ALAN]         = "assets/alan.ase",
    [SPRITE_BACKGROUND]   = "assets/background.ase",
    [SPRITE_BON_BON]      = "assets/bon_bon.ase",
    [SPRITE_BOOSTERS]     = "assets/boosters.ase",
    [SPRITE_BULLET]       = "assets/bullet.png",
    [SPRITE_ENEMY_BULLET] = "assets/enemy_bullet.ase",
    [SPRITE_EXPLOSION]    = "assets/explosion.ase",
    [SPRITE_GAME_OVER]    = "assets/gameover.png",
    [SPRITE_LIFE_ICON]    = "assets/life_icon.png",
    [SPRITE_LIPS]         = "assets/lips.ase",
    [SPRITE_PLAYER]       = "assets/player.ase",
};

static inline bool has_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot == NULL || dot == filename) { return false; }

    return strcmp(dot + 1, extension) == 0;
}

bool load_sprite(CF_Sprite* sprite, const char* path) {
    CF_ASSERT(sprite);
    CF_ASSERT(path);

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

void prefetch_sprites() {
    for (size_t i = 0; i < SPRITE_COUNT; ++i) { cf_draw_prefetch(&s_sprites[i]); }
}

bool load_sprites() {
    for (size_t i = 0; i < SPRITE_COUNT; ++i) {
        if (!load_sprite(&s_sprites[i], s_sprite_files[i])) return false;
    }

    return true;
}

CF_Sprite  get_sprite(const Sprite sprite) { return s_sprites[sprite]; }
CF_Sprite* get_sprite_ptr(const Sprite sprite) { return &s_sprites[sprite]; }

void render_sprite(CF_Sprite* sprite, const CF_V2 position, const ZIndex z_index) {
    cf_sprite_update(sprite);
    cf_draw() {
        cf_draw_layer(z_index) {
            cf_draw_translate_v2(position);
            cf_draw_sprite(sprite);
        }
    }
}
