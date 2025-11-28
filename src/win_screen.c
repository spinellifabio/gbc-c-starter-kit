#include <gb/gb.h>
#include <stdio.h>

#include "utils.h"
#include "win_screen.h"

void show_win_screen(uint16_t score) {
    // Clear screen and hide sprites to focus on the message
    clear_screen();
    HIDE_SPRITES;

    print_centered("YOU WIN!", 6u);
    print_centered("TREASURE SECURED", 8u);

    if (score > 0u) {
        char score_str[6];
        int_to_str(score, score_str, 5u);

        print_centered("SCORE:", 10u);

        gotoxy(13u, 10u);
        for (uint8_t i = 0u; i < 5u; i++) {
            printf("%c", score_str[i]);
        }
    }

    // Brief pause so accidental button mashing does not skip the screen
    uint8_t wait_frames = 180u;  // ~3 seconds at 60 FPS
    while (wait_frames--) {
        wait_vbl_done();
    }

    print_centered("PRESS START", 14u);
    wait_any_key();
}
