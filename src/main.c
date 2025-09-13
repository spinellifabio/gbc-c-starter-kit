/**
 *     $$$$$$\  $$$$$$$\   $$$$$$\         $$$$$$\         $$$$$$\    $$\                          $$\                               $$\   $$\ $$\   $$\
 *    $$  __$$\ $$  __$$\ $$  __$$\       $$  __$$\       $$  __$$\   $$ |                         $$ |                              $$ | $$  |\__|  $$ |
 *    $$ /  \__|$$ |  $$ |$$ /  \__|      $$ /  \__|      $$ /  \__|$$$$$$\    $$$$$$\   $$$$$$\ $$$$$$\    $$$$$$\   $$$$$$\        $$ |$$  / $$\ $$$$$$\
 *    $$ |$$$$\ $$$$$$$\ |$$ |            $$ |            \$$$$$$\  \_$$  _|   \____$$\ $$  __$$\\_$$  _|  $$  __$$\ $$  __$$\       $$$$$  /  $$ |\_$$  _|
 *    $$ |\_$$ |$$  __$$\ $$ |            $$ |             \____$$\   $$ |     $$$$$$$ |$$ |  \__| $$ |    $$$$$$$$ |$$ |  \__|      $$  $$<   $$ |  $$ |
 *    $$ |  $$ |$$ |  $$ |$$ |  $$\       $$ |  $$\       $$\   $$ |  $$ |$$\ $$  __$$ |$$ |       $$ |$$\ $$   ____|$$ |            $$ |\$$\  $$ |  $$ |$$\
 *    \$$$$$$  |$$$$$$$  |\$$$$$$  |      \$$$$$$  |      \$$$$$$  |  \$$$$  |\$$$$$$$ |$$ |       \$$$$  |\$$$$$$$\ $$ |            $$ | \$$\ $$ |  \$$$$  |
 *     \______/ \_______/  \______/        \______/        \______/    \____/  \_______|\__|        \____/  \_______|\__|            \__|  \__|\__|   \____/
 *
 *                                                                                                           v0.1.0 - Code assembled in spare time by Fabio
 */

 // main.c - Template minimo con game loop + VBlank (compatibile con SDCC/GBDK)
 // Compila con gbdk-2020; evita C99 compound literals (non supportati da sdcc).

#include <gb/gb.h>
#include <gb/cgb.h>
#include <gbdk/font.h>
#include <gbdk/console.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lang.h"
#include "gameplay.h"

/* -------------------- Palette -------------------- */
static const palette_color_t PALETTE0[4] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK };

/* -------------------- Game Mode ----------------- */
typedef enum {
    MODE_RELEASE,
    MODE_DEBUG
} GameMode;

/* -------------------- Global Settings ------------ */
typedef struct {
    uint8_t sound_on;
    uint8_t difficulty;
    uint8_t lives;
    GameMode mode;
    Language language;

    const char* game_name;
    const char* version;
} GameSettings;

static GameSettings settings = {
    1,                 /* sound_on */
    1,                 /* difficulty: 0=easy,1=normal,2=hard */
    3,                 /* lives */
    MODE_DEBUG,
    LANG_EN,
    "GBC Prototype",
    "v0.1.0"
};

/* -------------------- Menu System ----------------
 * Cambiamo la callback: ritorna 1 se serve ridisegnare TUTTA la schermata.
 */
typedef uint8_t(*MenuChangeFn)(int dir);
typedef struct {
    LangStringId label_id;
    MenuChangeFn change;
} MenuItem;

static uint8_t toggle_sound(int dir) { (void)dir; settings.sound_on ^= 1; return 0; }
static uint8_t cycle_difficulty(int dir) {
    if (dir > 0) settings.difficulty = (settings.difficulty + 1) % 3;
    else         settings.difficulty = (settings.difficulty + 2) % 3;
    return 0;
}
static uint8_t cycle_lives(int dir) {
    if (dir > 0) settings.lives = (settings.lives % 9) + 1;
    else         settings.lives = (settings.lives == 1 ? 9 : settings.lives - 1);
    return 0;
}
static uint8_t cycle_language(int dir) {
    if (dir > 0) settings.language = (settings.language + 1) % LANG_COUNT;
    else         settings.language = (settings.language == 0 ? (LANG_COUNT - 1) : settings.language - 1);
    lang_set(settings.language);
    /* → Le etichette e i testi cambiano: richiedi redraw completo */
    return 1;
}
static uint8_t toggle_mode(int dir) {
    (void)dir;
    settings.mode = (settings.mode == MODE_RELEASE ? MODE_DEBUG : MODE_RELEASE);
    return 0;
}

/* Etichette ora sono ID localizzati */
static MenuItem option_items[] = {
    { STR_OPT_SOUND,      toggle_sound     },
    { STR_OPT_DIFFICULTY, cycle_difficulty },
    { STR_OPT_LIVES,      cycle_lives      },
    { STR_OPT_MODE,       toggle_mode      },
    { STR_OPT_LANGUAGE,   cycle_language   }
};
#define OPTION_COUNT (sizeof(option_items) / sizeof(MenuItem))

/* -------------------- Input State ---------------- */
static uint8_t old_keys = 0;

static uint8_t get_pressed(void) {
    uint8_t keys = joypad();
    uint8_t pressed = keys & (uint8_t)(~old_keys);
    old_keys = keys;
    return pressed;
}

static void flush_input(void) {
    while (joypad()) wait_vbl_done();
    old_keys = 0;
}

/* -------------------- Helpers UI -----------------
 * Per stringhe “blink” di lunghezze diverse EN/IT, puliamo sempre 16 char.
 */
static void draw_blink_line(uint8_t x, uint8_t y, LangStringId id, uint8_t visible) {
    gotoxy(x, y);
    printf("                "); /* 16 spazi */
    if (visible) { gotoxy(x, y); printf("%s", lang_str(id)); }
}

/* -------------------- Splash --------------------- */
static void show_splash(LangStringId id_text, uint16_t duration_frames) {
    const char* text = lang_str(id_text);
    gotoxy((uint8_t)((20 - strlen(text)) / 2), 9);
    printf("%s", text);
    for (uint16_t f = 0; f < duration_frames; f++) wait_vbl_done();
    cls();
}
static void splash_sequence(void) {
    show_splash(STR_SPLASH_STUDIO, 120);
    show_splash(STR_SPLASH_PRESENTS, 120);
}

/* -------------------- Options -------------------- */
static void draw_option_line(uint8_t i, uint8_t cursor) {
    gotoxy(2, (uint8_t)(6 + i * 2));
    printf("%s", (i == cursor) ? ">" : " ");
    printf(" %s: ", lang_str(option_items[i].label_id));

    switch (i) {
    case 0: /* SOUND */
        printf("%s", lang_str(settings.sound_on ? STR_VAL_ON : STR_VAL_OFF));
        printf("   "); /* padding */
        break;

    case 1: /* DIFFICULTY */
        switch (settings.difficulty) {
        case 0: printf("%s", lang_str(STR_VAL_EASY));   break;
        case 1: printf("%s", lang_str(STR_VAL_NORMAL)); break;
        default:printf("%s", lang_str(STR_VAL_HARD));   break;
        }
        printf("      ");
        break;

    case 2: /* LIVES */
        printf("%d   ", settings.lives);
        break;

    case 3: /* MODE */
        printf("%s", lang_str(settings.mode == MODE_RELEASE ? STR_VAL_MODE_RELEASE : STR_VAL_MODE_DEBUG));
        printf("     ");
        break;

    case 4: /* LANGUAGE */
        printf("%s", lang_str(settings.language == LANG_EN ? STR_VAL_LANG_EN : STR_VAL_LANG_IT));
        printf("  ");
        break;
    }
}

static void draw_options(uint8_t cursor) {
    cls();
    gotoxy(6, 1);  printf("%s", lang_str(STR_OPTIONS_TITLE));
    gotoxy(2, 3);  printf(lang_str(STR_OPTIONS_BUILD), settings.game_name, settings.version);
    for (uint8_t i = 0; i < OPTION_COUNT; i++) draw_option_line(i, cursor);
    gotoxy(2, 16); printf("%s", lang_str(STR_OPTIONS_FOOTER_BACK));
}

static void options_screen(void) {
    uint8_t cursor = 0;
    draw_options(cursor);
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_B) break;

        if ((pressed & J_UP) && (cursor > 0)) {
            uint8_t old_cursor = cursor; cursor--;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if ((pressed & J_DOWN) && (cursor < (OPTION_COUNT - 1))) {
            uint8_t old_cursor = cursor; cursor++;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }

        if (pressed & (J_LEFT | J_RIGHT)) {
            int dir = (pressed & J_LEFT) ? -1 : +1;
            uint8_t full_redraw = option_items[cursor].change(dir);
            if (full_redraw) {
                /* Lingua cambiata → ridisegna TUTTO per aggiornare etichette/testi */
                draw_options(cursor);
            } else {
                /* Cambiata solo la voce corrente */
                draw_option_line(cursor, cursor);
            }
        }
    }
    flush_input();
    cls();
}

/* -------------------- Dialogue System RPG -------------------- */
#define DIALOG_BOX_Y 12
#define DIALOG_BOX_H 6

static void draw_dialog_box(void) {
    for (uint8_t y = DIALOG_BOX_Y; y < (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H); y++) {
        gotoxy(0, y);
        for (uint8_t x = 0; x < 20; x++) printf(" ");
    }
    gotoxy(0, DIALOG_BOX_Y);                     printf("--------------------");
    gotoxy(0, (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H - 1)); printf("--------------------");
    gotoxy(18, (uint8_t)(DIALOG_BOX_Y + DIALOG_BOX_H - 2)); printf("▶");
}

static uint8_t show_dialog_page_idx(uint8_t page_index) {
    draw_dialog_box();
    uint8_t hurry = 0;
    const uint8_t lines = lang_dialog_line_count(page_index);

    for (uint8_t l = 0; l < lines; l++) {
        const char* text = lang_dialog_line(page_index, l);
        uint8_t len = (uint8_t)strlen(text);
        for (uint8_t i = 0; i < len; i++) {
            gotoxy((uint8_t)(1 + i), (uint8_t)(DIALOG_BOX_Y + 1 + l));
            printf("%c", text[i]);

            uint8_t delay = (joypad() & J_A) ? 1u : 3u;
            for (uint8_t f = 0; f < delay; f++) {
                wait_vbl_done();
                if (joypad() & J_START) { hurry = 1; break; }
            }
            if (hurry) break;
        }
        if (hurry) break;
    }
    return hurry;
}

static void play_dialogue_sequence(void) {
    const uint8_t page_count = lang_dialog_page_count();
    for (uint8_t p = 0; p < page_count; p++) {
        if (show_dialog_page_idx(p)) break;
    }
    cls();
}

/* -------------------- Cutscene con dialogo ------------------- */
static void intro_cut_scene(void) {
    cls();

    uint16_t frame_counter = 0;
    uint8_t dot_state = 0;
    uint8_t hurry = 0;
    while (frame_counter < 120) { /* ~2s */
        wait_vbl_done();
        frame_counter++;
        if ((frame_counter % 20u) == 0u) {
            dot_state = (uint8_t)((dot_state + 1u) % 4u);
            gotoxy(9, 8);
            switch (dot_state) {
            case 0: printf("    "); break;
            case 1: printf(".   ");  break;
            case 2: printf("..  ");  break;
            default:printf("... ");  break;
            }
        }
        if (joypad() & J_START) { hurry = 1; break; }
    }
    if (!hurry) {
        play_dialogue_sequence();
    } else {
        cls();
        const char* hurry_text = lang_str(STR_INTRO_HURRY);
        gotoxy((uint8_t)((20 - strlen(hurry_text)) / 2), 9);
        printf("%s", hurry_text);
        for (uint16_t f = 0; f < 120; f++) wait_vbl_done();
    }
    flush_input();
    cls();
}

/* -------------------- Title ---------------------- */
static void title_screen(void) {
    uint8_t frame_counter = 0;
    uint8_t visible = 1;
    cls();

    set_bkg_tiles(5, 6, (uint8_t)strlen(settings.game_name), 1, (uint8_t*)settings.game_name);
    set_bkg_tiles(7, 7, (uint8_t)strlen(settings.version), 1, (uint8_t*)settings.version);

    while (1) {
        if (++frame_counter >= 30u) {
            frame_counter = 0;
            visible = (uint8_t)!visible;
            draw_blink_line(4, 11, STR_TITLE_HINT_START_PLAY, visible);
            draw_blink_line(4, 13, STR_TITLE_HINT_SELECT_OPT, visible);
        }
        uint8_t pressed = get_pressed();
        if (pressed & J_START) break;
        if (pressed & J_SELECT) options_screen();
        wait_vbl_done();
    }
    flush_input();
    cls();
}

/* -------------------- Main ----------------------- */
void main(void) {
    cgb_compatibility();
    DISPLAY_OFF;

    SHOW_BKG;
    font_init();
    font_set(font_load(font_ibm));
    set_bkg_palette(0, 1, PALETTE0);
    set_sprite_palette(0, 1, PALETTE0);
    DISPLAY_ON;

    lang_init(settings.language);

    splash_sequence();
    intro_cut_scene();

    while (1) {
        title_screen();
        gameplay_screen();
    }
}

/*
CPU/Timing:
- update_input(): ~ a poche dozzine di cicli
- wait_vbl_done(): blocca fino a LY >= 144 (VBlank), mantenendo frame pacing
- Se update_game()+draw() restano < ~9000-10000 cicli (≈1.1-1.3 ms @ 8.38 MHz eff),
  si mantiene 60 FPS stabile su GBC.

ROM/RAM:
- ROM: ~0.2–0.5 KB per questo file (dipende dall’ottimizzazione) + 8 byte palette
- RAM: 1 byte (`running`); palette in ROM (const).
*/
