#pragma once

#define CF_CONCAT(a, b) a##b
#define CF_VAR(line)    CF_CONCAT(_cf_guard_, line)
#define CF_SCOPE(push_call, pop_call)                                                             \
    for (int CF_VAR(__LINE__) = (push_call, 1); CF_VAR(__LINE__); CF_VAR(__LINE__) = 0, pop_call)

#define cf_draw()            CF_SCOPE(cf_draw_push(), cf_draw_pop())
#define cf_draw_color(color) CF_SCOPE(cf_draw_push_color(color), cf_draw_pop_color())
#define cf_draw_layer(layer) CF_SCOPE(cf_draw_push_layer(layer), cf_draw_pop_layer())
#define cf_font(font)        CF_SCOPE(cf_push_font(font), cf_pop_font())
