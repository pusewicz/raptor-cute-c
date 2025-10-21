#include "platform_cute.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_init.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>
#include <cute_alloc.h>
#include <cute_app.h>
#include <cute_c_runtime.h>
#include <cute_file_system.h>
#include <cute_result.h>
#include <cute_symbol.h>
#include <stddef.h>
#include <stdint.h>

#include "../engine/common.h"
#include "../engine/log.h"

constexpr int MAX_PATH_LENGTH = 1024;

#ifdef HOT_RELOAD_ENABLED
#ifndef GAME_LIBRARY_NAME
    #error "GAME_LIBRARY_NAME must be defined"
#endif

char         game_library_path[MAX_PATH_LENGTH] = {0};
SDL_PathInfo path_info;
#endif

static void mount_content_directory_as(const char* dir) {
    const char* path = cf_fs_get_base_directory();
    cf_path_normalize(path);
    char full_path[MAX_PATH_LENGTH];
    SDL_snprintf(full_path, MAX_PATH_LENGTH, "%s%s", path, "assets");
    APP_INFO("Mounting content directory %s as %s\n", full_path, dir);
    cf_fs_mount(full_path, dir, true);
}

void platform_init(const char* argv0) {
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, "Raptor");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, "0.1.0");
    SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_TYPE_STRING, "game");
#ifdef DEBUG
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);
#endif

    const int window_width  = 180;
    const int window_height = 320;
    const int options       = CF_APP_OPTIONS_WINDOW_POS_CENTERED_BIT | CF_APP_OPTIONS_RESIZABLE_BIT;
    CF_Result result = cf_make_app("Raptor", cf_default_display(), 0, 0, window_width, window_height, options, argv0);

    if (cf_is_error(result)) {
        APP_FATAL("Could not make app: %s", result.details);
        CF_ASSERT(false);
    }

    mount_content_directory_as("/assets");
}

void platform_shutdown(void) { cf_destroy_app(); }

void* platform_allocate_memory(size_t size) { return cf_calloc(size, 1); }
void  platform_free_memory(void* p) { cf_free(p); }

void platform_begin_frame(void) {}
void platform_end_frame(void) { cf_app_draw_onto_screen(true); }

#ifdef HOT_RELOAD_ENABLED
GameLibrary platform_load_game_library(void) {
    GameLibrary game_library = {0};

    const char* base_path    = SDL_GetBasePath();
    if (!base_path) {
        APP_FATAL("Failed to get base path: %s\n", SDL_GetError());
        CF_ASSERT(false);
    }
    const char* game_library_name = GAME_LIBRARY_NAME;

    SDL_snprintf(game_library_path, countof(game_library_path), "%s%s", base_path, game_library_name);

    if (!SDL_GetPathInfo(game_library_path, &path_info)) {
        APP_FATAL("Failed to get path info for %s: %s\n", game_library_path, SDL_GetError());
        CF_ASSERT(false);
    }

    game_library.path    = game_library_path;

    game_library.library = cf_load_shared_library(game_library.path);
    if (!game_library.library) {
        APP_ERROR("Failed to load library: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.init = (GameInitFunction)cf_load_function(game_library.library, "game_init");
    if (!game_library.init) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.update = (GameUpdateFunction)cf_load_function(game_library.library, "game_update");
    if (!game_library.update) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.render = (GameRenderFunction)cf_load_function(game_library.library, "game_render");
    if (!game_library.render) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.shutdown = (GameShutdownFunction)cf_load_function(game_library.library, "game_shutdown");
    if (!game_library.shutdown) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.state = (GameStateFunction)cf_load_function(game_library.library, "game_state");
    if (!game_library.state) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.hot_reload = (GameHotReloadFunction)cf_load_function(game_library.library, "game_hot_reload");
    if (!game_library.hot_reload) {
        APP_WARN("Failed to load function: %s\n", SDL_GetError());
        return game_library;
    }

    game_library.ok = true;
    APP_INFO("Game library loaded from %s.\n", game_library.path);
    return game_library;
}

void platform_unload_game_library(GameLibrary* game_library) {
    APP_DEBUG("Unloading library %s\n", game_library->path);
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
#endif // HOT_RELOAD_ENABLED

uint64_t platform_get_performance_counter(void) { return SDL_GetPerformanceCounter(); }
uint64_t platform_get_performance_frequency(void) { return SDL_GetPerformanceFrequency(); }
