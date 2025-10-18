#include "movement.h"

#include <cute_math.h>

void update_movement(CF_V2* position, CF_V2* velocity) {
    position->x += velocity->x;
    position->y += velocity->y;
}
