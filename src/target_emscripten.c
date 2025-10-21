#include <cute_app.h>
#include <cute_color.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <debugbreak.h>
#include <emscripten.h>
#include <stdio.h>

#include "engine/log.h"
#include "engine/platform.h"
#include "platform/platform_cute.h"

// Forward declarations of game functions (statically linked for Emscripten)
extern void  game_init(Platform* platform);
extern bool  game_update(void);
extern void* game_render(void);
extern void  game_shutdown(void);
extern void* game_state(void);
extern void  game_hot_reload(void* game_state);

static void debug_handler(bool expr, const char* message, const char* file, int line) {
    if (!expr) {
        fprintf(stderr, "CF_ASSERT(%s) : %s, line %d\n", message, file, line);
        debug_break();
    }
}

typedef struct MainLoopData {
    bool running;
} MainLoopData;

static MainLoopData main_loop_data = {
    .running = true,
};

static void main_loop(void* arg) {
    (void)arg;

    if (!cf_app_is_running()) {
        main_loop_data.running = false;
        emscripten_cancel_main_loop();
        return;
    }

    game_update();
    platform_begin_frame();
    game_render();
    platform_end_frame();
}

int main(int argc, char* argv[]) {
    (void)argc;

    platform_init(argv[0]);

    Platform platform = {
        .allocate_memory = platform_allocate_memory,
        .free_memory     = platform_free_memory,
    };

    game_init(&platform);

    CF_Color bg = cf_make_color_rgb(0, 0, 0);
    cf_clear_color(bg.r, bg.g, bg.b, bg.a);
    cf_set_target_framerate(60);
    cf_set_fixed_timestep(60);
    cf_set_assert_handler(debug_handler);

    // Emscripten requires using emscripten_set_main_loop instead of a while loop
    // 0 means use the browser's requestAnimationFrame (typically 60 FPS)
    // 1 means simulate an infinite loop
    emscripten_set_main_loop_arg(main_loop, &main_loop_data, 0, 1);

    // This code will only run when the main loop exits
    game_shutdown();
    platform_shutdown();

    return 0;
}
