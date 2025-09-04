#include "engine/common.h"
#include "engine/game_input.h"
#include "engine/game_memory.h"
#include "platform/platform.h"

#include <cute.h>

typedef struct UpdateData {
  GameLibrary *game_library;
  GameMemory  *memory;
  GameInput   *input;
} UpdateData;

void update(void *udata) {
  UpdateData *update_data  = (UpdateData *)udata;
  GameLibrary game_library = *(GameLibrary *)update_data->game_library;
  game_library.update(update_data->memory, update_data->input);
}

int main(int argc, char *argv[]) {
  (void)argc;

  platform_init(argv[0]);

  const int  size   = MiB(512);
  GameMemory memory = {
      .bytes = platform_allocate_memory(size),
      .size  = size,
  };
  GameInput input = {0};

  GameLibrary game_library = platform_open_game_library();

  /* UpdateData update_data = { */
  /*   .game_library = &game_library, */
  /*   .memory = &memory, */
  /*   .input = &input, */
  /* }; */

  game_library.init(&memory);

  CF_Color bg = cf_make_color_rgb(100, 149, 237);
  cf_clear_color(bg.r, bg.g, bg.b, bg.a);

  /* cf_set_update_udata(&update_data); */

  while (cf_app_is_running()) {
    platform_begin_frame();
    {
      cf_app_update(nullptr);
      /* game_library.update(&memory, &input); */
    }
    platform_end_frame();
  }

  game_library.shutdown(&memory);

  platform_close_game_library(&game_library);
  platform_shutdown();

  platform_free_memory(memory.bytes);

  return 0;
}
