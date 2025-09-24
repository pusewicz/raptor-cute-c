#pragma once

#include <SDL3/SDL_log.h>

#ifndef APP_DEACTIVATE_LOGGING

  #if DEBUG
    #define APP_TRACE(...) SDL_LogTrace(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
    #define APP_DEBUG(...) SDL_LogDebug(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
  #else
    #define APP_TRACE(...)
    #define APP_DEBUG(...)
  #endif

  #define APP_INFO(...)  SDL_LogInfo(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
  #define APP_WARN(...)  SDL_LogWarn(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
  #define APP_ERROR(...) SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)
  #define APP_FATAL(...) SDL_LogCritical(SDL_LOG_CATEGORY_CUSTOM, __VA_ARGS__)

#else

  #define APP_TRACE(...)
  #define APP_DEBUG(...)
  #define APP_INFO(...)
  #define APP_WARN(...)
  #define APP_ERROR(...)
  #define APP_FATAL(...)

#endif
