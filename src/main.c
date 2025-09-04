#include "engine/platform.h"
#include "platform/platform_cute.h"

#include <SDL3/SDL.h>
#include <cute_app.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <stddef.h>

typedef struct UpdateData {
  GameLibrary *game_library;
} UpdateData;

static void update(void *udata) {
  UpdateData  *update_data  = (UpdateData *)udata;
  GameLibrary *game_library = update_data->game_library;
  game_library->update();
}

int main(int argc, char *argv[]) {
  (void)argc;

  platform_init(argv[0]);

  Platform platform = {
      .allocate_memory = platform_allocate_memory,
      .free_memory     = platform_free_memory,
  };

  GameLibrary game_library = platform_load_game_library();

  UpdateData update_data = {
      .game_library = &game_library,
  };

  game_library.init(&platform);

  CF_Color bg = cf_make_color_rgb(100, 149, 237);
  cf_clear_color(bg.r, bg.g, bg.b, bg.a);
  cf_set_target_framerate(60);
  cf_set_fixed_timestep(60);
  cf_app_set_vsync(true);
  cf_set_update_udata(&update_data);

  while (cf_app_is_running()) {
    cf_app_update(&update);

    if (platform_has_to_reload_game_library(&game_library)) {
      void *game_state = game_library.state();
      platform_unload_game_library(&game_library);

      cf_sleep(50);
      GameLibrary new_game_library = platform_load_game_library();
      if (new_game_library.ok) {
        game_library = new_game_library;
        game_library.hot_reload(game_state);
      }
    }

    platform_begin_frame();
    game_library.render();
    platform_end_frame();
  }

  game_library.shutdown();

  platform_unload_game_library(&game_library);
  platform_shutdown();

  return 0;
}
