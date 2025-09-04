#include "../engine/common.h"
#include "platform.h"

#include <SDL3/SDL.h>
#include <cute.h>
#include <cute_symbol.h>

#define MAX_PATH_LENGTH 1024

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

void *platform_allocate_memory(int size) { return cf_calloc(size, 1); }
void  platform_free_memory(void *p) { cf_free(p); }

void platform_begin_frame(void) {};
void platform_end_frame(void) { cf_app_draw_onto_screen(true); }

GameLibrary platform_open_game_library(void) {
  const char *base_path = SDL_GetBasePath();
  if (!base_path) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get base path: %s\n", SDL_GetError());
    abort();
  }
#ifdef SDL_PLATFORM_WIN32
  const char *game_library_name      = "gamelib.dll";
  const char *game_library_copy_name = "gamelib_copy.dll";
  SDL_snprintf(game_library_copy_path, MAX_PATH_LENGTH, "%s%s", base_path, game_library_copy_name);
#elif SDL_PLATFORM_LINUX
  const char *game_library_name = "libgamelib.so";
#elif SDL_PLATFORM_MACOS
  const char *game_library_name = "libgamelib.dylib";
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

  GameLibrary game_library = {0};

#ifdef SDL_PLATFORM_WIN32
  game_library.library_name = game_library_copy_path;
#else
  game_library.library_name = game_library_path;
#endif

  game_library.library  = cf_load_shared_library(game_library.library_name);
  game_library.init     = (GameInitFunction)cf_load_function(game_library.library, "game_init");
  game_library.update   = (GameUpdateFunction)cf_load_function(game_library.library, "game_update");
  game_library.render   = (GameRenderFunction)cf_load_function(game_library.library, "game_render");
  game_library.shutdown = (GameShutdownFunction)cf_load_function(game_library.library, "game_shutdown");

  SDL_Log("Game library loaded.\n");
  return game_library;
}

void platform_close_game_library(GameLibrary *game_library) { cf_unload_shared_library(game_library->library); }

void platform_reload_game_library(GameLibrary *game_library) {
  SDL_PathInfo new_path_info;
  if (!SDL_GetPathInfo(game_library_path, &new_path_info)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to get path info: %s\n", SDL_GetError());
  }

  if (new_path_info.modify_time != path_info.modify_time) {
    cf_unload_shared_library(game_library->library);
    game_library->shutdown = NULL;
    game_library->render   = NULL;
    game_library->update   = NULL;
    game_library->init     = NULL;
    game_library->library  = NULL;

    SDL_Delay(50);

#ifdef SDL_PLATFORM_WIN32
    if (!SDL_CopyFile(game_library_path, game_library_copy_path)) {
      SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Failed to copy dynamic library: %s\n", SDL_GetError());
    }
#endif

    game_library->library  = cf_load_shared_library(game_library->library_name);
    game_library->init     = (GameInitFunction)cf_load_function(game_library->library, "game_init");
    game_library->update   = (GameUpdateFunction)cf_load_function(game_library->library, "game_update");
    game_library->render   = (GameRenderFunction)cf_load_function(game_library->library, "game_render");
    game_library->shutdown = (GameShutdownFunction)cf_load_function(game_library->library, "game_shutdown");

    path_info = new_path_info;
    SDL_Log("Game library reloaded.\n");
  }
}

uint64_t platform_get_performance_counter(void) { return SDL_GetPerformanceCounter(); }
uint64_t platform_get_performance_frequency(void) { return SDL_GetPerformanceFrequency(); }
