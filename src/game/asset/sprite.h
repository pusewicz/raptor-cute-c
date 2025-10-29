#pragma once

typedef struct CF_Sprite CF_Sprite;
typedef struct CF_V2     CF_V2;
typedef enum ZIndex      ZIndex;
typedef enum Sprite {
    SPRITE_BOOSTERS,
    SPRITE_BULLET,
    SPRITE_ENEMY_BULLET,
    SPRITE_EXPLOSION,
    SPRITE_GAME_OVER,
    SPRITE_LIFE_ICON,
    SPRITE_PLAYER,
    SPRITE_ALAN,
    SPRITE_BON_BON,
    SPRITE_LIPS,
    SPRITE_BACKGROUND,
    SPRITE_COUNT,
} Sprite;

bool       load_sprite(CF_Sprite* sprite, const char* path);
CF_Sprite  get_sprite(const Sprite sprite);
CF_Sprite* get_sprite_ptr(const Sprite sprite);
bool       load_sprites();
void       prefetch_sprites();
void       render_sprite(CF_Sprite* sprite, const CF_V2 position, const ZIndex z_index);
