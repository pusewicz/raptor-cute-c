#pragma once

typedef struct CF_Sprite CF_Sprite;
typedef struct CF_V2     CF_V2;
typedef enum ZIndex      ZIndex;
typedef enum Sprite {
    SPRITE_ALAN,
    SPRITE_BACKGROUND,
    SPRITE_BON_BON,
    SPRITE_BOOSTERS,
    SPRITE_BULLET,
    SPRITE_ENEMY_BULLET,
    SPRITE_EXPLOSION,
    SPRITE_GAME_OVER,
    SPRITE_LIFE_ICON,
    SPRITE_LIPS,
    SPRITE_PLAYER,
    SPRITE_COUNT,
} Sprite;

CF_Sprite  load_sprite(const char* path);
CF_Sprite  get_sprite(const Sprite sprite);
CF_Sprite* get_sprite_ptr(const Sprite sprite);
void       load_sprites();
void       prefetch_sprites();
void       render_sprite(CF_Sprite* sprite, const CF_V2 position, const ZIndex z_index);
