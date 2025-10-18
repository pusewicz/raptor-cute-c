#pragma once

typedef struct CF_Sprite CF_Sprite;
typedef struct CF_V2     CF_V2;
typedef enum ZIndex      ZIndex;

void render_sprite(CF_Sprite* sprite, CF_V2 position, ZIndex z_index);
