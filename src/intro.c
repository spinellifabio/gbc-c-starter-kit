#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dialogue.h"
#include "input.h"
#include "intro.h"
#include "lang.h"

void intro_cut_scene(void) {
    cls();

    uint16_t frame_counter = 0u;
    uint8_t dot_state = 0u;
    uint8_t hurry = 0u;
    while (frame_counter < 120u) {
        wait_vbl_done();
        frame_counter++;
        if ((frame_counter % 20u) == 0u) {
            dot_state = (uint8_t)((dot_state + 1u) % 4u);
            gotoxy(9u, 8u);
            switch (dot_state) {
            case 0u: printf("    "); break;
            case 1u: printf(".   ");  break;
            case 2u: printf("..  ");  break;
            default: printf("... ");  break;
            }
        }
        if (joypad() & J_START) {
            hurry = 1u;
            break;
        }
    }
    if (!hurry) {
        play_dialogue_sequence();
    } else {
        cls();
        const char *hurry_text = lang_str(STR_INTRO_HURRY);
        gotoxy((uint8_t)((20u - strlen(hurry_text)) / 2u), 9u);
        printf("%s", hurry_text);
        for (uint16_t f = 0; f < 120u; f++) {
            wait_vbl_done();
        }
    }
    flush_input();
    cls();
}
