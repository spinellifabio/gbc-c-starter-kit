#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "input.h"
#include "lang.h"
#include "save.h"
#include "slot_menu.h"
#include "utils.h"

/* Screen layout (20x18 tiles) */
#define SLOT_ROW0       5u    /* first slot row; slots at 5, 7, 9 */
#define SLOT_ROW_STEP   2u
#define ACTION_ROW0     12u   /* action submenu rows: 12, 13, 14 */
#define FOOTER_ROW      16u

/* Action submenu indices (VALID slot) */
enum { ACT_CONTINUE = 0, ACT_OVERWRITE, ACT_ERASE, ACT_COUNT };

static const LangStringId ACTION_LABELS[ACT_COUNT] = {
    STR_SLOT_CONTINUE, STR_SLOT_OVERWRITE, STR_SLOT_ERASE
};

static char diff_char(uint8_t difficulty) {
    switch (difficulty) {
    case 0u:  return 'E';
    case 1u:  return 'N';
    default:  return 'H';
    }
}

/* Clears columns 2..17 — a 2-column margin on each side so text doesn't sit
 * flush against the screen edge. Uses text_char_tile() — a raw ' ' (0x20) is
 * the wrong tile index whenever the font doesn't load at tile 0, which is
 * what produced the garbled/junk tiles on this screen (font base is
 * runtime-determined; see include/utils.h). */
static void clear_row(uint8_t row) {
    fill_bkg_rect(2u, row, 16u, 1u, text_char_tile(' '));
    bg_row_highlight(row, 2u, 16u, 0u);
}

static void clear_rows(uint8_t start, uint8_t count) {
    for (uint8_t i = 0u; i < count; i++) {
        clear_row((uint8_t)(start + i));
    }
}

static void print_clipped(const char *s, uint8_t max_len) {
    uint8_t len = (uint8_t)strlen(s);
    if (len > max_len) {
        len = max_len;
    }
    for (uint8_t i = 0u; i < len; i++) {
        printf("%c", s[i]);
    }
}

/* Draw one slot row with its cursor marker and status summary. Status text is
 * clipped to the remaining row budget (16-col margin, see clear_row) — a
 * high score (up to 5 digits) must never push text past col 17. */
static void draw_slot_line(uint8_t i, uint8_t cursor) {
    SaveSlot s;
    SaveStatus status = save_slot_read(i, &s);
    uint8_t row = (uint8_t)(SLOT_ROW0 + i * SLOT_ROW_STEP);
    const char *label = lang_str(STR_SLOT_LABEL);

    clear_row(row);
    gotoxy(2u, row);
    printf("%c %s %d ", (i == cursor) ? '>' : ' ', label, (int)(i + 1u));

    uint8_t prefix_len = (uint8_t)(3u + strlen(label) + 3u); /* "> "+label+" N " */
    uint8_t budget = (prefix_len < 16u) ? (uint8_t)(16u - prefix_len) : 0u;

    char status_buf[16];
    switch (status) {
    case SAVE_VALID: {
        char lives_buf[4];
        char score_buf[6];
        int_to_str(s.lives, lives_buf, 1u);
        int_to_str(s.score, score_buf, 1u);
        uint8_t p = 0u;
        status_buf[p++] = 'L';
        for (uint8_t k = 0u; lives_buf[k]; k++) status_buf[p++] = lives_buf[k];
        status_buf[p++] = ' ';
        status_buf[p++] = diff_char(s.difficulty);
        status_buf[p++] = ' ';
        status_buf[p++] = 'S';
        for (uint8_t k = 0u; score_buf[k]; k++) status_buf[p++] = score_buf[k];
        status_buf[p] = '\0';
        break;
    }
    case SAVE_CORRUPTED:
        strcpy(status_buf, lang_str(STR_SLOT_CORRUPTED));
        break;
    default:
        strcpy(status_buf, lang_str(STR_SLOT_EMPTY));
        break;
    }
    print_clipped(status_buf, budget);

    bg_row_highlight(row, 2u, 16u, (i == cursor) ? 1u : 0u);
}

/* Repaint only the cursor marker on a slot row (no SRAM re-read). */
static void draw_cursor(uint8_t i, uint8_t cursor) {
    uint8_t row = (uint8_t)(SLOT_ROW0 + i * SLOT_ROW_STEP);
    gotoxy(2u, row);
    printf("%c", (i == cursor) ? '>' : ' ');
    bg_row_highlight(row, 2u, 16u, (i == cursor) ? 1u : 0u);
}

static void draw_footer(void) {
    clear_row(FOOTER_ROW);
    gotoxy(2u, FOOTER_ROW);
    print_clipped(lang_str(STR_SLOT_FOOTER), 16u);
}

static void draw_all(uint8_t cursor) {
    cls();
    clear_attr_map();
    print_centered(lang_str(STR_SLOT_TITLE), 1u);
    for (uint8_t i = 0u; i < SAVE_SLOT_COUNT; i++) {
        draw_slot_line(i, cursor);
    }
    draw_footer();
}

/* Modal yes/no prompt. Returns 1 if confirmed (A), 0 if cancelled (B). */
static uint8_t confirm_prompt(void) {
    clear_rows(ACTION_ROW0, 3u);
    print_centered(lang_str(STR_SLOT_CONFIRM), (uint8_t)(ACTION_ROW0 + 1u));
    flush_input();
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_A) { sfx_play(SFX_UI_SELECT); return 1u; }
        if (pressed & J_B) { sfx_play(SFX_UI_BACK);   return 0u; }
    }
}

/* Draw the 3-action submenu for a VALID slot. */
static void draw_actions(uint8_t cursor) {
    for (uint8_t a = 0u; a < ACT_COUNT; a++) {
        uint8_t row = (uint8_t)(ACTION_ROW0 + a);
        clear_row(row);
        gotoxy(4u, row);
        printf("%c %s", (a == cursor) ? '>' : ' ', lang_str(ACTION_LABELS[a]));
        bg_row_highlight(row, 2u, 16u, (a == cursor) ? 1u : 0u);
    }
}

static void clear_actions(void) {
    clear_rows(ACTION_ROW0, ACT_COUNT);
}

/* Handle a VALID slot. Returns 1 if the player chose CONTINUE, else 0
 * (overwrite/erase performed, or backed out — caller redraws the list). */
static uint8_t handle_valid_slot(uint8_t i) {
    uint8_t cursor = 0u;
    draw_actions(cursor);
    flush_input();
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();

        if (pressed & J_B) {
            sfx_play(SFX_UI_BACK);
            clear_actions();
            return 0u;
        }
        if ((pressed & J_UP) && (cursor > 0u)) {
            cursor--;
            sfx_play(SFX_UI_MOVE);
            draw_actions(cursor);
        }
        if ((pressed & J_DOWN) && (cursor < (ACT_COUNT - 1u))) {
            cursor++;
            sfx_play(SFX_UI_MOVE);
            draw_actions(cursor);
        }
        if (pressed & J_A) {
            sfx_play(SFX_UI_SELECT);
            if (cursor == ACT_CONTINUE) {
                clear_actions();
                return 1u;
            } else if (cursor == ACT_OVERWRITE) {
                if (confirm_prompt()) {
                    save_autosave(i);
                }
                clear_actions();
                return 0u;
            } else { /* ACT_ERASE — double confirm */
                if (confirm_prompt() && confirm_prompt()) {
                    save_slot_erase(i);
                }
                clear_actions();
                return 0u;
            }
        }
    }
}

SlotChoice slot_menu(void) {
    SlotChoice choice;
    uint8_t cursor = 0u;

    WY_REG = 144u;
    HIDE_WIN;
    set_solid_bkg(BKG_COLOR_OPTIONS);
    draw_all(cursor);
    flush_input();

    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();

        if (pressed & J_B) {
            sfx_play(SFX_UI_BACK);
            choice.slot = cursor;
            choice.action = SLOT_ACTION_CANCEL;
            break;
        }
        if ((pressed & J_UP) && (cursor > 0u)) {
            uint8_t old = cursor;
            cursor--;
            sfx_play(SFX_UI_MOVE);
            draw_cursor(old, cursor);
            draw_cursor(cursor, cursor);
        }
        if ((pressed & J_DOWN) && (cursor < (SAVE_SLOT_COUNT - 1u))) {
            uint8_t old = cursor;
            cursor++;
            sfx_play(SFX_UI_MOVE);
            draw_cursor(old, cursor);
            draw_cursor(cursor, cursor);
        }
        if (pressed & J_A) {
            sfx_play(SFX_UI_SELECT);
            if (save_slot_status(cursor) == SAVE_VALID) {
                if (handle_valid_slot(cursor)) {
                    choice.slot = cursor;
                    choice.action = SLOT_ACTION_CONTINUE;
                    break;
                }
                /* overwrite/erase/back: refresh list and footer */
                draw_all(cursor);
                flush_input();
            } else {
                /* EMPTY or CORRUPTED → start fresh (first save overwrites) */
                choice.slot = cursor;
                choice.action = SLOT_ACTION_NEW;
                break;
            }
        }
    }

    flush_input();
    cls();
    clear_attr_map();
    return choice;
}
