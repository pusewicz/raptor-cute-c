#pragma once

#include <string.h>

static inline bool has_extension(const char* filename, const char* extension) {
    const char* dot = strrchr(filename, '.');
    if (dot == nullptr || dot == filename) { return false; }

    return strcmp(dot + 1, extension) == 0;
}
