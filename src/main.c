#include "engine/game_memory.h"
#include "engine/game_state.h"
#include "platform/platform.h"

#include <cute_app.h>
#include <cute_color.h>
#include <cute_draw.h>
#include <cute_graphics.h>
#include <cute_time.h>
#include <stddef.h>

typedef struct UpdateData {
  GameLibrary *game_library;
  GameMemory  *memory;
} UpdateData;

static void update(void *udata) {
  UpdateData  *update_data  = (UpdateData *)udata;
  GameLibrary *game_library = update_data->game_library;
  game_library->update(update_data->memory);
}

int main(int argc, char *argv[]) {
  (void)argc;

  platform_init(argv[0]);

  const size_t size   = sizeof(GameState);
  GameMemory   memory = {
        .bytes = platform_allocate_memory(size),
        .size  = size,
  };

  GameLibrary game_library = platform_open_game_library();

  UpdateData update_data = {
      .game_library = &game_library,
      .memory       = &memory,
  };

  game_library.init(&memory);

  CF_Color bg = cf_make_color_rgb(100, 149, 237);
  cf_clear_color(bg.r, bg.g, bg.b, bg.a);
  cf_set_target_framerate(60);
  cf_set_fixed_timestep(60);
  cf_set_update_udata(&update_data);

  while (cf_app_is_running()) {
    platform_reload_game_library(&game_library);
    platform_begin_frame();
    {
      cf_app_update(&update);
      game_library.render(&memory);
    }
    platform_end_frame();
  }

  game_library.shutdown(&memory);

  platform_close_game_library(&game_library);
  platform_shutdown();

  platform_free_memory(memory.bytes);

  return 0;
}
