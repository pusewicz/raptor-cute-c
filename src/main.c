#ifndef ENGINE_ENABLE_HOT_RELOAD
    #define ENGINE_ENABLE_HOT_RELOAD 0
#endif

#include <cute_app.h>
#include <cute_color.h>
#include <cute_defines.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <debugbreak.h>
#include <stdio.h>
#ifdef ENGINE_ENABLE_HOT_RELOAD
    #include <signal.h>
    #include <sys/signal.h>
#endif

#ifdef CF_EMSCRIPTEN
    #include <emscripten.h>
#endif

#include "engine/log.h"
#include "engine/platform.h"
#include "platform/platform_cute.h"

constexpr const int TARGET_FPS = 60;

#if ENGINE_ENABLE_HOT_RELOAD
volatile sig_atomic_t reload_flag = 0;

static void sighup_handler(int sig) {
    (void)sig;
    reload_flag = 1;
}

static void debug_handler(bool expr, const char* message, const char* file, int line) {
    if (!expr) {
        fprintf(stderr, "CF_ASSERT(%s) : %s, line %d\n", message, file, line);
        debug_break();
    }
}
#endif  // ENGINE_ENABLE_HOT_RELOAD

static void on_cf_app_update(void* udata) {
    GameLibrary* game_library = (GameLibrary*)udata;
    game_library->update();
}

static void update(void* udata) {
    GameLibrary* game_library = (GameLibrary*)udata;
    cf_app_update(&on_cf_app_update);

#if ENGINE_ENABLE_HOT_RELOAD
    if (reload_flag == 1) {
        reload_flag = 0;
        APP_DEBUG("Reloading library %s\n", game_library->path);

        void* state = game_library->state();
        platform_unload_game_library(game_library);

        GameLibrary new_game_library = platform_load_game_library();
        if (new_game_library.ok) {
            *game_library = new_game_library;
            game_library->hot_reload(state);
        }
    }
#endif  // ENGINE_ENABLE_HOT_RELOAD

    platform_begin_frame();
    game_library->render();
    platform_end_frame();
}

int main(int argc [[maybe_unused]], char* argv[]) {
#if ENGINE_ENABLE_HOT_RELOAD
    signal(SIGHUP, sighup_handler);
#endif  // ENGINE_ENABLE_HOT_RELOAD

    platform_init(argv[0]);

    Platform platform = {
        .allocate_memory = platform_allocate_memory,
        .free_memory     = platform_free_memory,
    };
    GameLibrary game_library = platform_load_game_library();
    game_library.init(&platform);

    CF_Color bg = cf_make_color_rgb(0, 0, 0);
    cf_clear_color(bg.r, bg.g, bg.b, bg.a);
    cf_set_target_framerate(TARGET_FPS);
    cf_set_fixed_timestep(TARGET_FPS);
    cf_app_set_vsync(true);
    cf_set_update_udata(&game_library);

#if ENGINE_ENABLE_HOT_RELOAD
    cf_set_assert_handler(debug_handler);
#endif  // ENGINE_ENABLE_HOT_RELOAD

#ifdef CF_EMSCRIPTEN
    emscripten_set_main_loop_arg(update, &game_library, TARGET_FPS, true);
#else
    while (cf_app_is_running()) { update(&game_library); }
#endif

    game_library.shutdown();

    platform_unload_game_library(&game_library);
    platform_shutdown();

    return 0;
}
