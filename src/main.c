#include <cute_app.h>
#include <cute_color.h>
#include <cute_defines.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <debugbreak.h>
#include <stdio.h>
#ifndef CF_EMSCRIPTEN
    #include <signal.h>
    #include <sys/signal.h>
#endif

#ifdef CF_EMSCRIPTEN
    #include <emscripten.h>
#endif

#include "engine/log.h"
#include "engine/platform.h"
#include "game/game.h"
#include "platform/platform_cute.h"

constexpr const int UPDATE_INTERVAL = 60;

#ifndef CF_EMSCRIPTEN
volatile sig_atomic_t reload_flag = 0;

static void sighup_handler(int sig) {
    (void)sig;
    reload_flag = 1;
}
#endif

static void debug_handler(bool expr, const char* message, const char* file, int line) {
    if (!expr) {
        fprintf(stderr, "CF_ASSERT(%s) : %s, line %d\n", message, file, line);
        debug_break();
    }
}

typedef struct UpdateData {
    GameLibrary* game_library;
} UpdateData;

static void on_update(void* udata [[maybe_unused]]) {
#ifndef CF_EMSCRIPTEN
    UpdateData*  update_data  = (UpdateData*)udata;
    GameLibrary* game_library = update_data->game_library;
    game_library->update();
#else
    game_update();
#endif
}

static void update(void* gl [[maybe_unused]]) {
    cf_app_update(&on_update);

#ifndef CF_EMSCRIPTEN
    GameLibrary* game_library = (GameLibrary*)gl;
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
#endif

    platform_begin_frame();
#ifndef CF_EMSCRIPTEN
    game_library->render();
#else
    game_render();
#endif
    platform_end_frame();
}

int main(int argc, char* argv[]) {
    (void)argc;

#ifndef CF_EMSCRIPTEN
    signal(SIGHUP, sighup_handler);
#endif

    platform_init(argv[0]);

    Platform platform = {
        .allocate_memory = platform_allocate_memory,
        .free_memory     = platform_free_memory,
    };
#ifndef CF_EMSCRIPTEN
    GameLibrary game_library = platform_load_game_library();
    UpdateData  update_data  = {
          .game_library = &game_library,
    };

    game_library.init(&platform);
#else
    UpdateData update_data = {0};
    game_init(&platform);
#endif

    CF_Color bg = cf_make_color_rgb(0, 0, 0);
    cf_clear_color(bg.r, bg.g, bg.b, bg.a);
    cf_set_target_framerate(UPDATE_INTERVAL);
    cf_set_fixed_timestep(UPDATE_INTERVAL);
    cf_app_set_vsync(true);
    cf_set_update_udata(&update_data);
    cf_set_assert_handler(debug_handler);

#ifdef CF_EMSCRIPTEN
    emscripten_set_main_loop_arg(update, nullptr, UPDATE_INTERVAL, true);
#else
    while (cf_app_is_running()) { update(&game_library); }
#endif

#ifndef CF_EMSCRIPTEN
    game_library.shutdown();
    platform_unload_game_library(&game_library);
#else
    game_shutdown();
#endif

    platform_shutdown();

    return 0;
}
