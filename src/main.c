#include <cute_app.h>
#include <cute_color.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <debugbreak.h>
#include <stdio.h>

#include "engine/log.h"
#include "engine/platform.h"
#include "platform/platform_cute.h"

#ifdef HOT_RELOAD_ENABLED
#include <signal.h>
#include <sys/signal.h>

volatile sig_atomic_t reload_flag = 0;

static void sighup_handler(int sig) {
    (void)sig;
    reload_flag = 1;
}
#else
// In Release mode, directly include game functions
#include "game/game.h"
#endif

static void debug_handler(bool expr, const char* message, const char* file, int line) {
    if (!expr) {
        fprintf(stderr, "CF_ASSERT(%s) : %s, line %d\n", message, file, line);
        debug_break();
    }
}

#ifdef HOT_RELOAD_ENABLED
typedef struct UpdateData {
    GameLibrary* game_library;
} UpdateData;

static void update(void* udata) {
    UpdateData*  update_data  = (UpdateData*)udata;
    GameLibrary* game_library = update_data->game_library;
    game_library->update();
}
#else
static void update(void* udata) {
    (void)udata;
    game_update();
}
#endif

int main(int argc, char* argv[]) {
    (void)argc;

    platform_init(argv[0]);

    Platform platform = {
        .allocate_memory = platform_allocate_memory,
        .free_memory     = platform_free_memory,
    };

#ifdef HOT_RELOAD_ENABLED
    signal(SIGHUP, sighup_handler);

    GameLibrary game_library = platform_load_game_library();
    UpdateData  update_data  = {
          .game_library = &game_library,
    };

    game_library.init(&platform);
#else
    game_init(&platform);
#endif

    CF_Color bg = cf_make_color_rgb(0, 0, 0);
    cf_clear_color(bg.r, bg.g, bg.b, bg.a);
    cf_set_target_framerate(60);
    cf_set_fixed_timestep(60);
    cf_app_set_vsync(true);
#ifdef HOT_RELOAD_ENABLED
    cf_set_update_udata(&update_data);
#else
    cf_set_update_udata(NULL);
#endif
    cf_set_assert_handler(debug_handler);

    while (cf_app_is_running()) {
        cf_app_update(&update);

#ifdef HOT_RELOAD_ENABLED
        if (reload_flag == 1) {
            reload_flag = 0;
            APP_DEBUG("Reloading library %s\n", game_library.path);

            void* state = game_library.state();
            platform_unload_game_library(&game_library);

            GameLibrary new_game_library = platform_load_game_library();
            if (new_game_library.ok) {
                game_library = new_game_library;
                game_library.hot_reload(state);
            }
        }

        platform_begin_frame();
        game_library.render();
        platform_end_frame();
#else
        platform_begin_frame();
        game_render();
        platform_end_frame();
#endif
    }

#ifdef HOT_RELOAD_ENABLED
    game_library.shutdown();
    platform_unload_game_library(&game_library);
#else
    game_shutdown();
#endif

    platform_shutdown();

    return 0;
}
