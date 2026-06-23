#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>

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

static void clear_row(uint8_t row) {
    fill_bkg_rect(0u, row, 20u, 1u, (uint8_t)' ');  /* single DMA-friendly fill */
}

static void clear_rows(uint8_t start, uint8_t count) {
    for (uint8_t i = 0u; i < count; i++) {
        clear_row((uint8_t)(start + i));
    }
}

/* Draw one slot row with its cursor marker and status summary. */
static void draw_slot_line(uint8_t i, uint8_t cursor) {
    SaveSlot s;
    SaveStatus status = save_slot_read(i, &s);
    uint8_t row = (uint8_t)(SLOT_ROW0 + i * SLOT_ROW_STEP);

    clear_row(row);
    gotoxy(1u, row);
    printf("%c %s %d ", (i == cursor) ? '>' : ' ', lang_str(STR_SLOT_LABEL), (int)(i + 1u));

    switch (status) {
    case SAVE_VALID:
        printf("L%d %c S%04d", (int)s.lives, diff_char(s.difficulty), (int)s.score);
        break;
    case SAVE_CORRUPTED:
        printf("%s", lang_str(STR_SLOT_CORRUPTED));
        break;
    default:
        printf("%s", lang_str(STR_SLOT_EMPTY));
        break;
    }
}

/* Repaint only the cursor marker on a slot row (no SRAM re-read). */
static void draw_cursor(uint8_t i, uint8_t cursor) {
    gotoxy(1u, (uint8_t)(SLOT_ROW0 + i * SLOT_ROW_STEP));
    printf("%c", (i == cursor) ? '>' : ' ');
}

static void draw_footer(void) {
    clear_row(FOOTER_ROW);
    gotoxy(1u, FOOTER_ROW);
    printf("%s", lang_str(STR_SLOT_FOOTER));
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
        gotoxy(3u, row);
        printf("%c %s", (a == cursor) ? '>' : ' ', lang_str(ACTION_LABELS[a]));
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
