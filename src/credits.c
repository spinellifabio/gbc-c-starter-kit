#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "credits.h"
#include "input.h"

#define SCREEN_WIDTH_TILES 20u
#define SCREEN_HEIGHT_TILES 18u
#define SCROLL_DELAY_FRAMES 8u

static const char BLANK_ROW[SCREEN_WIDTH_TILES + 1u] = {
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
    '\0'
};

static const char * const CREDIT_LINES[] = {
    "",
    "",
    "CREDITS",
    "",
    "Programming",
    "Fabio Spinelli",
    "",
    "Framework",
    "GBDK-2020",
    "",
    "Sample Assets",
    "Alex & Friends",
    "",
    "Special Thanks",
    "Game Boy Devs",
    "",
    "",
    "See you next time!"
};

#define CREDIT_LINE_COUNT ((uint8_t)(sizeof(CREDIT_LINES) / sizeof(CREDIT_LINES[0])))

static void render_rows(int16_t top_row) {
    for (uint8_t row = 0u; row < SCREEN_HEIGHT_TILES; row++) {
        int16_t line_index = top_row + (int16_t)row;

        gotoxy(0u, row);
        printf("%s", BLANK_ROW);

        if ((line_index >= 0) && (line_index < CREDIT_LINE_COUNT)) {
            const char *line = CREDIT_LINES[(uint8_t)line_index];
            uint8_t len = (uint8_t)strlen(line);
            if (len > SCREEN_WIDTH_TILES) {
                len = SCREEN_WIDTH_TILES;
            }
            if (len > 0u) {
                uint8_t x = (uint8_t)((SCREEN_WIDTH_TILES - len) >> 1);
                gotoxy(x, row);
                printf("%s", line);
            }
        }
    }
}

void credits_scene(void) {
    int16_t top_row = -(int16_t)SCREEN_HEIGHT_TILES;
    uint8_t frame_counter = 0u;
    uint8_t done = 0u;

    flush_input();

    HIDE_WIN;
    SCX_REG = 0u;
    SCY_REG = 0u;
    cls();
    render_rows(top_row);

    while (!done) {
        wait_vbl_done();

        uint8_t pressed = get_pressed();
        if (pressed & (J_START | J_SELECT)) {
            done = 1u;
            break;
        }

        if (++frame_counter >= SCROLL_DELAY_FRAMES) {
            frame_counter = 0u;
            top_row++;
            if (top_row > (int16_t)(CREDIT_LINE_COUNT + SCREEN_HEIGHT_TILES)) {
                done = 1u;
            } else {
                render_rows(top_row);
            }
        }
    }

    flush_input();
    cls();
}
