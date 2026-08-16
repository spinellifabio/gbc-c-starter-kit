#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "audio.h"
#include "bgm.h"
#include "game_settings.h"
#include "input.h"
#include "lang.h"
#include "options_screen.h"
#include "utils.h"

typedef uint8_t (*MenuChangeFn)(int dir);

typedef struct {
    LangStringId label_id;
    MenuChangeFn change;
} MenuItem;

static uint8_t toggle_sound(int dir) {
    (void)dir;
    g_settings.sound_on ^= 1u;
    audio_set_enabled(g_settings.sound_on);
    return 0u;
}

static uint8_t cycle_difficulty(int dir) {
    if (dir > 0) {
        g_settings.difficulty = (uint8_t)((g_settings.difficulty + 1u) % 3u);
    } else {
        g_settings.difficulty = (uint8_t)((g_settings.difficulty + 2u) % 3u);
    }
    return 0u;
}

static uint8_t cycle_lives(int dir) {
    if (dir > 0) {
        g_settings.lives = (uint8_t)((g_settings.lives % 9u) + 1u);
    } else {
        g_settings.lives = (g_settings.lives == 1u) ? 9u : (uint8_t)(g_settings.lives - 1u);
    }
    return 0u;
}

static uint8_t cycle_language(int dir) {
    if (dir > 0) {
        g_settings.language = (Language)((g_settings.language + 1u) % LANG_COUNT);
    } else {
        g_settings.language = (g_settings.language == 0u)
            ? (Language)(LANG_COUNT - 1u)
            : (Language)(g_settings.language - 1u);
    }
    lang_set(g_settings.language);
    return 1u;
}

static uint8_t toggle_mode(int dir) {
    (void)dir;
    g_settings.mode = (g_settings.mode == MODE_RELEASE) ? MODE_DEBUG : MODE_RELEASE;
    return 0u;
}

static MenuItem option_items[] = {
    { STR_OPT_SOUND,      toggle_sound     },
    { STR_OPT_DIFFICULTY, cycle_difficulty },
    { STR_OPT_LIVES,      cycle_lives      },
    { STR_OPT_MODE,       toggle_mode      },
    { STR_OPT_LANGUAGE,   cycle_language   }
};

#define OPTION_COUNT (sizeof(option_items) / sizeof(MenuItem))

/* Row layout: cursor glyph at column OPT_ROW_X, row content clipped to
 * OPT_ROW_WIDTH columns so cursor+label+": "+value can never wrap past the
 * 20-column screen (FR-008). A longer localized value is truncated, not
 * wrapped (spec Edge Cases). 2-column margin on each side so text doesn't
 * sit flush against the screen edge. */
#define OPT_ROW_X     2u
#define OPT_ROW_WIDTH 16u /* columns 2..17 inclusive */

static void print_clipped(const char *s, uint8_t max_len) {
    uint8_t len = (uint8_t)strlen(s);
    if (len > max_len) {
        len = max_len;
    }
    for (uint8_t i = 0u; i < len; i++) {
        printf("%c", s[i]);
    }
}

static void draw_option_line(uint8_t i, uint8_t cursor) {
    uint8_t row = (uint8_t)(6u + i * 2u);

    /* Clear the row's content span first so a shorter new value never
     * leaves a stale tail from a longer previous one. */
    fill_bkg_rect(OPT_ROW_X, row, OPT_ROW_WIDTH, 1u, text_char_tile(' '));
    gotoxy(OPT_ROW_X, row);
    printf("%c", (i == cursor) ? '>' : ' ');

    char value[10];
    switch (i) {
    case 0u:
        strcpy(value, lang_str(g_settings.sound_on ? STR_VAL_ON : STR_VAL_OFF));
        break;
    case 1u:
        switch (g_settings.difficulty) {
        case 0u: strcpy(value, lang_str(STR_VAL_EASY));   break;
        case 1u: strcpy(value, lang_str(STR_VAL_NORMAL)); break;
        default: strcpy(value, lang_str(STR_VAL_HARD));   break;
        }
        break;
    case 2u:
        int_to_str(g_settings.lives, value, 1u);
        break;
    case 3u:
        strcpy(value, lang_str(g_settings.mode == MODE_RELEASE ? STR_VAL_MODE_RELEASE : STR_VAL_MODE_DEBUG));
        break;
    case 4u:
        strcpy(value, lang_str(g_settings.language == LANG_EN ? STR_VAL_LANG_EN : STR_VAL_LANG_IT));
        break;
    default:
        value[0] = '\0';
        break;
    }

    const char *label = lang_str(option_items[i].label_id);
    uint8_t budget = (uint8_t)(OPT_ROW_WIDTH - 1u); /* minus the cursor glyph */
    uint8_t label_len = (uint8_t)strlen(label);
    if (label_len > budget) {
        label_len = budget;
    }
    print_clipped(label, label_len);
    budget = (uint8_t)(budget - label_len);

    if (budget >= 2u) {
        printf(": ");
        budget = (uint8_t)(budget - 2u);
        print_clipped(value, budget);
    }
    /* budget < 2: no room even for ": " — label alone already fills the row. */

    bg_row_highlight(row, OPT_ROW_X, OPT_ROW_WIDTH, (i == cursor) ? 1u : 0u);
}

static void draw_options(uint8_t cursor) {
    cls();
    clear_attr_map();
    gotoxy(6u, 1u);
    printf("%s", lang_str(STR_OPTIONS_TITLE));

    /* Build-info line: print+clip directly (no sprintf into a fixed buffer —
     * g_settings.game_name/version are unbounded const char*) so a long
     * name/version pair truncates instead of overflowing a stack buffer or
     * wrapping past column 19. STR_OPTIONS_BUILD is "%s %s" in every
     * language, so the space separator is safe to hardcode here. */
    gotoxy(OPT_ROW_X, 3u);
    uint8_t build_budget = OPT_ROW_WIDTH;
    uint8_t name_len = (uint8_t)strlen(g_settings.game_name);
    if (name_len > build_budget) {
        name_len = build_budget;
    }
    print_clipped(g_settings.game_name, name_len);
    build_budget = (uint8_t)(build_budget - name_len);
    if (build_budget >= 1u) {
        printf(" ");
        build_budget--;
        print_clipped(g_settings.version, build_budget);
    }

    for (uint8_t i = 0u; i < OPTION_COUNT; i++) {
        draw_option_line(i, cursor);
    }
    gotoxy(OPT_ROW_X, 16u);
    print_clipped(lang_str(STR_OPTIONS_FOOTER_BACK), OPT_ROW_WIDTH);
}

void options_screen(void) {
    uint8_t cursor = 0u;
    bgm_pause();
    WY_REG = 144u;
    HIDE_WIN;
    set_solid_bkg(BKG_COLOR_OPTIONS);
    draw_options(cursor);
    flush_input();
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_B) {
            sfx_play(SFX_UI_BACK);
            break;
        }

        if ((pressed & J_UP) && (cursor > 0u)) {
            uint8_t old_cursor = cursor;
            cursor--;
            sfx_play(SFX_UI_MOVE);
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if ((pressed & J_DOWN) && (cursor < (OPTION_COUNT - 1u))) {
            uint8_t old_cursor = cursor;
            cursor++;
            sfx_play(SFX_UI_MOVE);
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }

        if (pressed & (J_LEFT | J_RIGHT)) {
            int dir = (pressed & J_LEFT) ? -1 : +1;
            sfx_play(SFX_UI_SELECT);
            uint8_t full_redraw = option_items[cursor].change(dir);
            if (full_redraw) {
                draw_options(cursor);
            } else {
                draw_option_line(cursor, cursor);
            }
        }
    }
    bgm_resume();
    flush_input();
    cls();
    clear_attr_map();
}
