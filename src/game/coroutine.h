#pragma once
#include <cute_coroutine.h>

void co_sleep(CF_Coroutine co, float seconds);
void enemy_spawner(CF_Coroutine co);
void init_coroutines(void);
void cleanup_coroutines(void);
