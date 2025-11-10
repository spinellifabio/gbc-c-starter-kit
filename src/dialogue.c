#include <gb/gb.h>
#include <gbdk/platform.h>

#include <stdint.h>
#include <string.h>

#include "dialogue.h"
#include "lang.h"

#define DIALOG_BOX_WIDTH          20u
#define DIALOG_BOX_HEIGHT         5u
#define INNER_DIALOGUE_BOX_WIDTH  (DIALOG_BOX_WIDTH - 2u)
#define INNER_DIALOGUE_BOX_HEIGHT (DIALOG_BOX_HEIGHT - 2u)

#define DIALOGUE_WIN_X            7u
#define DIALOGUE_WIN_ONSCREEN_Y   ((DEVICE_SCREEN_HEIGHT * 8u) - (DIALOG_BOX_HEIGHT * 8u))
#define DIALOGUE_WIN_OFFSCREEN_Y  (DEVICE_SCREEN_HEIGHT * 8u)

#define DIALOGUE_TEXT_BUFFER (LANG_DLG_PAGE_COUNT * LANG_DLG_MAX_LINES_PER_PAGE * (INNER_DIALOGUE_BOX_WIDTH + 4u))

static int16_t window_y_position = DIALOGUE_WIN_OFFSCREEN_Y;

static uint8_t char_to_tile(char c) {
    if ((c >= 'a') && (c <= 'z')) {
        c = (char)(c - ('a' - 'A'));
    }
    if ((uint8_t)c < ' ' || (uint8_t)c > 'Z') {
        c = ' ';
    }
    return (uint8_t)((uint8_t)c - ' ');
}

static void set_win_char(uint8_t x, uint8_t y, char c) {
    uint8_t tile = char_to_tile(c);
    set_win_tiles(x, y, 1u, 1u, &tile);
}

static void vsync_frames(uint8_t count) {
    while (count--) {
        wait_vbl_done();
    }
}

static void wait_for_advance(void) {
    uint8_t state;
    do {
        state = joypad();
        wait_vbl_done();
    } while (state & (J_A | J_START));
    do {
        state = joypad();
        wait_vbl_done();
    } while ((state & (J_A | J_START)) == 0u);
    do {
        state = joypad();
        wait_vbl_done();
    } while (state & (J_A | J_START));
}

static uint8_t is_alpha_numeric(char c) {
    if ((c >= 'a') && (c <= 'z')) return 1u;
    if ((c >= 'A') && (c <= 'Z')) return 1u;
    if ((c >= '0') && (c <= '9')) return 1u;
    return 0u;
}

static uint8_t should_break_line(const char *text, uint16_t index, uint8_t row_size) {
    if (row_size >= INNER_DIALOGUE_BOX_WIDTH) return 1u;
    char c = text[index];
    if (c == '\0') return 0u;
    if (is_alpha_numeric(c)) return 0u;

    uint8_t distance = (uint8_t)(row_size + 1u);
    while ((c = text[++index]) != '\0') {
        if (!is_alpha_numeric(c)) break;
        distance++;
    }
    return (uint8_t)(distance > INNER_DIALOGUE_BOX_WIDTH);
}

static void draw_dialogue_box(void) {
    set_win_char(0u, 0u, '+');
    for (uint8_t x = 1u; x < (uint8_t)(DIALOG_BOX_WIDTH - 1u); x++) {
        set_win_char(x, 0u, '-');
    }
    set_win_char((uint8_t)(DIALOG_BOX_WIDTH - 1u), 0u, '+');

    for (uint8_t y = 1u; y < (uint8_t)(DIALOG_BOX_HEIGHT - 1u); y++) {
        set_win_char(0u, y, '|');
        for (uint8_t x = 1u; x < (uint8_t)(DIALOG_BOX_WIDTH - 1u); x++) {
            set_win_char(x, y, ' ');
        }
        set_win_char((uint8_t)(DIALOG_BOX_WIDTH - 1u), y, '|');
    }

    set_win_char(0u, (uint8_t)(DIALOG_BOX_HEIGHT - 1u), '+');
    for (uint8_t x = 1u; x < (uint8_t)(DIALOG_BOX_WIDTH - 1u); x++) {
        set_win_char(x, (uint8_t)(DIALOG_BOX_HEIGHT - 1u), '-');
    }
    set_win_char((uint8_t)(DIALOG_BOX_WIDTH - 1u), (uint8_t)(DIALOG_BOX_HEIGHT - 1u), '+');
}

static void clear_dialogue_text(void) {
    for (uint8_t y = 1u; y <= INNER_DIALOGUE_BOX_HEIGHT; y++) {
        for (uint8_t x = 1u; x <= INNER_DIALOGUE_BOX_WIDTH; x++) {
            set_win_char(x, y, ' ');
        }
    }
}

static void slide_dialogue_box_on_screen(void) {
    window_y_position = DIALOGUE_WIN_OFFSCREEN_Y;
    move_win(DIALOGUE_WIN_X, (uint8_t)window_y_position);
    SHOW_WIN;
    while (window_y_position > DIALOGUE_WIN_ONSCREEN_Y) {
        window_y_position -= 8;
        move_win(DIALOGUE_WIN_X, (uint8_t)window_y_position);
        wait_vbl_done();
    }
}

static void slide_dialogue_box_off_screen(void) {
    while (window_y_position < DIALOGUE_WIN_OFFSCREEN_Y) {
        window_y_position += 8;
        move_win(DIALOGUE_WIN_X, (uint8_t)window_y_position);
        wait_vbl_done();
    }
    HIDE_WIN;
}

static void show_continue_arrow(void) {
    set_win_char((uint8_t)(DIALOG_BOX_WIDTH - 2u), (uint8_t)(DIALOG_BOX_HEIGHT - 2u), '>');
}

static void hide_continue_arrow(void) {
    set_win_char((uint8_t)(DIALOG_BOX_WIDTH - 2u), (uint8_t)(DIALOG_BOX_HEIGHT - 2u), ' ');
}

static void prompt_continue(void) {
    show_continue_arrow();
    wait_for_advance();
    hide_continue_arrow();
    clear_dialogue_text();
}

void dialogue_show_text(const char *text) {
    uint16_t index = 0u;
    uint8_t row = 0u;
    uint8_t column = 0u;
    uint8_t has_pending = 0u;

    draw_dialogue_box();
    clear_dialogue_text();
    hide_continue_arrow();
    slide_dialogue_box_on_screen();

    while (text[index] != '\0') {
        char c = text[index];
        index++;

        if (c == '\n') {
            column = 0u;
            row++;
        } else {
            set_win_char((uint8_t)(1u + column), (uint8_t)(1u + row), c);
            column++;
            has_pending = 1u;

            if (!should_break_line(text, index, column)) {
                uint8_t delay = (joypad() & J_A) ? 1u : 3u;
                vsync_frames(delay);
                continue;
            }

            column = 0u;
            row++;
        }

        while (text[index] == ' ') {
            index++;
        }

        if (row >= INNER_DIALOGUE_BOX_HEIGHT) {
            prompt_continue();
            row = 0u;
            column = 0u;
            has_pending = 0u;
        }
    }

    if (has_pending) {
        prompt_continue();
    } else {
        show_continue_arrow();
        wait_for_advance();
        hide_continue_arrow();
    }

    slide_dialogue_box_off_screen();
    clear_dialogue_text();
}

static void append_text(char *dest, size_t capacity, const char *src) {
    size_t len = strlen(dest);
    size_t room = (capacity > len) ? (capacity - len - 1u) : 0u;
    size_t i = 0u;
    while ((src[i] != '\0') && room) {
        char c = src[i++];
        dest[len++] = c;
        room--;
    }
    dest[len] = '\0';
}

void play_dialogue_sequence(void) {
    char dialogue_text[DIALOGUE_TEXT_BUFFER];
    dialogue_text[0] = '\0';

    const uint8_t page_count = lang_dialog_page_count();
    for (uint8_t p = 0u; p < page_count; p++) {
        const uint8_t line_count = lang_dialog_line_count(p);
        for (uint8_t l = 0u; l < line_count; l++) {
            append_text(dialogue_text, sizeof(dialogue_text), lang_dialog_line(p, l));
            if ((l + 1u) < line_count) {
                append_text(dialogue_text, sizeof(dialogue_text), " ");
            }
        }
        if ((p + 1u) < page_count) {
            append_text(dialogue_text, sizeof(dialogue_text), " ");
        }
    }

    dialogue_show_text(dialogue_text);
}
