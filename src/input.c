#include "input.h"

static uint8_t old_keys = 0;

uint8_t get_pressed(void) {
    uint8_t keys = joypad();
    uint8_t pressed = keys & (uint8_t)(~old_keys);
    old_keys = keys;
    return pressed;
}

void flush_input(void) {
    while (joypad()) {
        wait_vbl_done();
    }
    old_keys = 0;
}
