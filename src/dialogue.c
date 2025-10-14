#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "dialogue.h"
#include "lang.h"

#define DIALOG_BOX_Y 12u
#define DIALOG_BOX_H 6u

static void draw_dialog_box(void) {
    for (uint8_t y = DIALOG_BOX_Y; y < (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H); y++) {
        gotoxy(0u, y);
        for (uint8_t x = 0; x < 20u; x++) {
            printf(" ");
        }
    }
    gotoxy(0u, DIALOG_BOX_Y);
    printf("--------------------");
    gotoxy(0u, (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H - 1u));
    printf("--------------------");
    gotoxy(18u, (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H - 2u));
    printf("?");
}

static uint8_t show_dialog_page_idx(uint8_t page_index) {
    draw_dialog_box();
    uint8_t hurry = 0;
    const uint8_t lines = lang_dialog_line_count(page_index);

    for (uint8_t l = 0; l < lines; l++) {
        const char *text = lang_dialog_line(page_index, l);
        uint8_t len = (uint8_t)strlen(text);
        for (uint8_t i = 0; i < len; i++) {
            gotoxy((uint8_t)(1u + i), (uint8_t)(DIALOG_BOX_Y + 1u + l));
            printf("%c", text[i]);

            uint8_t delay = (joypad() & J_A) ? 1u : 3u;
            for (uint8_t f = 0; f < delay; f++) {
                wait_vbl_done();
                if (joypad() & J_START) {
                    hurry = 1u;
                    break;
                }
            }
            if (hurry) {
                break;
            }
        }
        if (hurry) {
            break;
        }
    }
    return hurry;
}

void play_dialogue_sequence(void) {
    const uint8_t page_count = lang_dialog_page_count();
    for (uint8_t p = 0; p < page_count; p++) {
        if (show_dialog_page_idx(p)) {
            break;
        }
    }
    cls();
}
