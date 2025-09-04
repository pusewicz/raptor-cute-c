#pragma once

#include <stddef.h>

typedef struct Platform {
  void *(*allocate_memory)(size_t size);
  void (*free_memory)(void *p);
} Platform;
