#pragma once

#define KiB(x) ((x) * 1024)
#define MiB(x) (KiB(x) * 1024)
#define GiB(x) (MiB(x) * 1024)

#ifdef DEBUG
  #if _MSC_VER
    #define assert(condition) \
      if (!(condition))       \
      __debugbreak()
  #else
    #define assert(condition) \
      if (!(condition))       \
      __builtin_trap()
  #endif
#else
  #define assert(condition)
#endif

#define countof(array)   (sizeof(array) / sizeof(array[0]))
#define lengthof(string) (countof(string) - 1)
