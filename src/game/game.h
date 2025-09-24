#pragma once

#include "../engine/platform.h"

#ifdef _WIN32
  #define EXPORT __declspec(dllexport)
#else
  #define EXPORT
#endif

typedef struct Platform Platform;

EXPORT void  game_init(Platform *platform);
EXPORT bool  game_update(void);
EXPORT void  game_render(void);
EXPORT void  game_shutdown(void);
EXPORT void *game_state(void);
EXPORT void  game_hot_reload(void *game_state);
