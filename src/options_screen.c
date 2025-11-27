#include <gb/gb.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>
#include <string.h>

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

static void draw_option_line(uint8_t i, uint8_t cursor) {
    gotoxy(2u, (uint8_t)(6u + i * 2u));
    printf("%s", (i == cursor) ? ">" : " ");
    printf(" %s: ", lang_str(option_items[i].label_id));

    switch (i) {
    case 0u:
        printf("%s", lang_str(g_settings.sound_on ? STR_VAL_ON : STR_VAL_OFF));
        printf("   ");
        break;
    case 1u:
        switch (g_settings.difficulty) {
        case 0u: printf("%s", lang_str(STR_VAL_EASY));   break;
        case 1u: printf("%s", lang_str(STR_VAL_NORMAL)); break;
        default: printf("%s", lang_str(STR_VAL_HARD));   break;
        }
        printf("      ");
        break;
    case 2u:
        printf("%d   ", g_settings.lives);
        break;
    case 3u:
        printf("%s", lang_str(g_settings.mode == MODE_RELEASE ? STR_VAL_MODE_RELEASE : STR_VAL_MODE_DEBUG));
        printf("     ");
        break;
    case 4u:
        printf("%s", lang_str(g_settings.language == LANG_EN ? STR_VAL_LANG_EN : STR_VAL_LANG_IT));
        printf("  ");
        break;
    default:
        break;
    }
}

static void draw_options(uint8_t cursor) {
    cls();
    clear_attr_map();
    gotoxy(6u, 1u);
    printf("%s", lang_str(STR_OPTIONS_TITLE));
    gotoxy(2u, 3u);
    printf(lang_str(STR_OPTIONS_BUILD), g_settings.game_name, g_settings.version);
    for (uint8_t i = 0u; i < OPTION_COUNT; i++) {
        draw_option_line(i, cursor);
    }
    gotoxy(2u, 16u);
    printf("%s", lang_str(STR_OPTIONS_FOOTER_BACK));
}

void options_screen(void) {
    uint8_t cursor = 0u;
    draw_options(cursor);
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_B) {
            break;
        }

        if ((pressed & J_UP) && (cursor > 0u)) {
            uint8_t old_cursor = cursor;
            cursor--;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if ((pressed & J_DOWN) && (cursor < (OPTION_COUNT - 1u))) {
            uint8_t old_cursor = cursor;
            cursor++;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }

        if (pressed & (J_LEFT | J_RIGHT)) {
            int dir = (pressed & J_LEFT) ? -1 : +1;
            uint8_t full_redraw = option_items[cursor].change(dir);
            if (full_redraw) {
                draw_options(cursor);
            } else {
                draw_option_line(cursor, cursor);
            }
        }
    }
    flush_input();
    cls();
    clear_attr_map();
}
