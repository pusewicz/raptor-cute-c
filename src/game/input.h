#pragma once

typedef struct Input {
    bool up;
    bool down;
    bool left;
    bool right;
    bool shoot;
} Input;

void update_input(Input* input);
