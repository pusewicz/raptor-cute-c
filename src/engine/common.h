#pragma once

#define countof(array)   (sizeof(array) / sizeof(array[0]))
#define lengthof(string) (countof(string) - 1)
