#include "platform_cute.h"

#include "../engine/common.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_platform_defines.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_result.h>
#include <cute_symbol.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_PATH_LENGTH 1024

#ifndef GAME_LIBRARY_NAME
  #error "GAME_LIBRARY_NAME must be defined"
#endif

char         game_library_path[MAX_PATH_LENGTH] = {0};
SDL_PathInfo path_info;

#ifdef SDL_PLATFORM_WIN32
char game_library_copy_path[MAX_PATH_LENGTH] = {0};
#endif

void platform_init(const char *argv0) {
#ifdef DEBUG
  SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#endif

  const int screen_width  = 1920;
  const int screen_height = 1080;
  const int options       = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT;

  CF_Result result = cf_make_app("Raptor", 0, 0, 0, screen_width, screen_height, options, argv0);
  if (cf_is_error(result)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Could not make app: %s", result.details);
    abort();
  }
}

void platform_shutdown(void) { cf_destroy_app(); }

void *platform_allocate_memory(size_t size) { return cf_calloc(size, 1); }
void  platform_free_memory(void *p) { cf_free(p); }

void platform_begin_frame(void) {}
void platform_end_frame(void) { cf_app_draw_onto_screen(true); }

GameLibrary platform_load_game_library(void) {
  GameLibrary game_library = {0};

  const char *base_path = SDL_GetBasePath();
  if (!base_path) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get base path: %s\n", SDL_GetError());
    abort();
  }
  const char *game_library_name = GAME_LIBRARY_NAME;
#ifdef SDL_PLATFORM_WIN32
  const char *game_library_copy_name = "copy_of" GAME_LIBRARY_NAME;
  SDL_snprintf(game_library_copy_path, MAX_PATH_LENGTH, "%s%s", base_path, game_library_copy_name);
#endif

  SDL_snprintf(game_library_path, countof(game_library_path), "%s%s", base_path, game_library_name);

  if (!SDL_GetPathInfo(game_library_path, &path_info)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get path info for %s: %s\n", game_library_path, SDL_GetError());
    abort();
  }

#ifdef SDL_PLATFORM_WIN32
  if (!SDL_CopyFile(game_library_path, game_library_copy_path)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to copy dynamic library: %s\n", SDL_GetError());
    abort();
  }
#endif

#ifdef SDL_PLATFORM_WIN32
  game_library.path = game_library_copy_path;
#else
  game_library.path = game_library_path;
#endif

  game_library.library = cf_load_shared_library(game_library.path);
  if (!game_library.library) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load library: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.init = (GameInitFunction)cf_load_function(game_library.library, "game_init");
  if (!game_library.init) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.update = (GameUpdateFunction)cf_load_function(game_library.library, "game_update");
  if (!game_library.update) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.render = (GameRenderFunction)cf_load_function(game_library.library, "game_render");
  if (!game_library.render) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.shutdown = (GameShutdownFunction)cf_load_function(game_library.library, "game_shutdown");
  if (!game_library.shutdown) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.state = (GameStateFunction)cf_load_function(game_library.library, "game_state");
  if (!game_library.state) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.hot_reload = (GameHotReloadFunction)cf_load_function(game_library.library, "game_hot_reload");
  if (!game_library.hot_reload) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to load function: %s\n", SDL_GetError());
    return game_library;
  }

  game_library.ok = true;
  SDL_Log("Game library loaded from %s.\n", game_library.path);
  return game_library;
}

void platform_unload_game_library(GameLibrary *game_library) {
  cf_unload_shared_library(game_library->library);
  game_library->hot_reload = nullptr;
  game_library->state      = nullptr;
  game_library->shutdown   = nullptr;
  game_library->render     = nullptr;
  game_library->update     = nullptr;
  game_library->init       = nullptr;
  game_library->library    = nullptr;
  game_library->ok         = false;
}

bool platform_has_to_reload_game_library(GameLibrary *game_library) {
  SDL_PathInfo new_path_info;
  if (!SDL_GetPathInfo(game_library->path, &new_path_info)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get path info: %s\n", SDL_GetError());
    return false;
  }

  return new_path_info.modify_time != path_info.modify_time;
}

void platform_reload_game_library(GameLibrary *game_library) {
  SDL_PathInfo new_path_info;
  if (!SDL_GetPathInfo(game_library_path, &new_path_info)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get path info: %s\n", SDL_GetError());
  }

  if (new_path_info.modify_time != path_info.modify_time) {
    cf_unload_shared_library(game_library->library);
    game_library->shutdown = nullptr;
    game_library->render   = nullptr;
    game_library->update   = nullptr;
    game_library->init     = nullptr;
    game_library->library  = nullptr;

    SDL_Delay(50);

#ifdef SDL_PLATFORM_WIN32
    if (!SDL_CopyFile(game_library_path, game_library_copy_path)) {
      SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to copy dynamic library: %s\n", SDL_GetError());
    }
#endif

    game_library->library  = cf_load_shared_library(game_library->path);
    game_library->init     = (GameInitFunction)cf_load_function(game_library->library, "game_init");
    game_library->update   = (GameUpdateFunction)cf_load_function(game_library->library, "game_update");
    game_library->render   = (GameRenderFunction)cf_load_function(game_library->library, "game_render");
    game_library->shutdown = (GameShutdownFunction)cf_load_function(game_library->library, "game_shutdown");

    path_info = new_path_info;
    SDL_Log("Game library reloaded from %s.\n", game_library->path);
  }
}

uint64_t platform_get_performance_counter(void) { return SDL_GetPerformanceCounter(); }
uint64_t platform_get_performance_frequency(void) { return SDL_GetPerformanceFrequency(); }
