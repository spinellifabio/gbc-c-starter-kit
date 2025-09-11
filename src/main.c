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

// -------------------- Palette --------------------
static const palette_color_t PALETTE0[4] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK };

// -------------------- Game Mode -----------------
typedef enum {
    MODE_RELEASE,
    MODE_DEBUG
} GameMode;

// -------------------- Global Settings ------------
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
    1,
    1,
    3,
    MODE_RELEASE,
    LANG_IT,
    "GBC Prototype",
    "v0.2.0" };

// -------------------- Menu System ----------------
typedef void (*MenuChangeFn)(int dir);
typedef struct {
    const char* label;
    MenuChangeFn change;
} MenuItem;

static void toggle_sound(int dir) { settings.sound_on ^= 1; }
static void cycle_difficulty(int dir) {
    if (dir > 0) settings.difficulty = (settings.difficulty + 1) % 3;
    else settings.difficulty = (settings.difficulty + 2) % 3;
}
static void cycle_lives(int dir) {
    if (dir > 0) settings.lives = (settings.lives % 9) + 1;
    else settings.lives = (settings.lives == 1 ? 9 : settings.lives - 1);
}
static void cycle_language(int dir) {
    if (dir > 0) settings.language = (settings.language + 1) % 2;
    else settings.language = (settings.language == 0 ? 1 : settings.language - 1);
    lang_set(settings.language);
}
static void toggle_mode(int dir) {
    settings.mode = (settings.mode == MODE_RELEASE ? MODE_DEBUG : MODE_RELEASE);
}

static MenuItem option_items[] = {
    {"SOUND", toggle_sound},
    {"DIFFICULTY", cycle_difficulty},
    {"LIVES", cycle_lives},
    {"MODE", toggle_mode},
    {"LANGUAGE", cycle_language}
};
#define OPTION_COUNT (sizeof(option_items) / sizeof(MenuItem))

// -------------------- Input State ----------------
static uint8_t old_keys = 0;

static uint8_t get_pressed(void) {
    uint8_t keys = joypad();
    uint8_t pressed = keys & ~old_keys;
    old_keys = keys;
    return pressed;
}

static void flush_input(void) {
    while (joypad()) wait_vbl_done();
    old_keys = 0;
}

// -------------------- Splash ---------------------
static void show_splash(const char* text, uint16_t duration_frames) {
    gotoxy((20 - strlen(text)) / 2, 9);
    printf(text);
    for (uint16_t f = 0; f < duration_frames; f++) wait_vbl_done();
    cls();
}
static void splash_sequence(void) {
    show_splash("OPENAI GAMES", 120);
    show_splash("PRESENTS", 120);
}

// -------------------- Options --------------------
static void draw_option_line(uint8_t i, uint8_t cursor) {
    gotoxy(2, 6 + i * 2);
    printf(i == cursor ? ">" : " ");
    printf(" %s: ", option_items[i].label);

    switch (i) {
    case 0: printf(settings.sound_on ? "ON " : "OFF"); break;
    case 1:
        switch (settings.difficulty) {
        case 0: printf("EASY   "); break;
        case 1: printf("NORMAL "); break;
        case 2: printf("HARD   "); break;
        }
        break;
    case 2: printf("%d", settings.lives); break;
    case 3: printf(settings.mode == MODE_RELEASE ? "RELEASE" : "DEBUG"); break;
    case 4: printf(settings.language == LANG_EN ? "EN" : "IT"); break;
    }
}
static void draw_options(uint8_t cursor) {
    cls();
    gotoxy(6, 1); printf("OPTIONS");
    gotoxy(2, 3); printf("%s %s", settings.game_name, settings.version);
    for (uint8_t i = 0; i < OPTION_COUNT; i++) draw_option_line(i, cursor);
    gotoxy(2, 16); printf("PRESS B TO RETURN");
}
static void options_screen(void) {
    uint8_t cursor = 0;
    draw_options(cursor);
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_B) break;
        if (pressed & J_UP && cursor > 0) {
            uint8_t old_cursor = cursor; cursor--;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_DOWN && cursor < OPTION_COUNT - 1) {
            uint8_t old_cursor = cursor; cursor++;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_LEFT) {
            option_items[cursor].change(-1);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_RIGHT) {
            option_items[cursor].change(1);
            draw_option_line(cursor, cursor);
        }
    }
    flush_input();
    cls();
}

// -------------------- Dialogue System RPG --------------------
#define DIALOG_BOX_Y 12
#define DIALOG_BOX_H 6

typedef struct {
    const char* lines[3]; // fino a 3 righe per pagina
    uint8_t line_count;
} DialoguePage;

// Dialoghi in 2 lingue
static const DialoguePage dialog_it[] = {
    {{"Benvenuto nel prototipo!", "Non e' molto ancora...", "Ma almeno i testi funzionano."}, 3},
    {{"Premi A per velocizzare.", "Premi START per saltare.", ""}, 2}
};
static const DialoguePage dialog_en[] = {
    {{"Welcome to the prototype!", "It's not much yet...", "But at least text works."}, 3},
    {{"Press A to speed up.", "Press START to skip.", ""}, 2}
};

// Disegna il box di dialogo
static void draw_dialog_box(void) {
    for (uint8_t y = DIALOG_BOX_Y; y < DIALOG_BOX_Y + DIALOG_BOX_H; y++) {
        gotoxy(0, y);
        for (uint8_t x = 0; x < 20; x++) printf(" ");
    }
    gotoxy(0, DIALOG_BOX_Y);
    printf("--------------------");
    gotoxy(0, DIALOG_BOX_Y + DIALOG_BOX_H - 1);
    printf("--------------------");

    // Indicatore visivo in basso a destra
    gotoxy(18, DIALOG_BOX_Y + DIALOG_BOX_H - 2);
    printf("▶");
}

// Stampa una pagina con effetto typewriter
static uint8_t show_dialog_page(const DialoguePage* page) {
    draw_dialog_box();
    uint8_t hurry = 0;

    for (uint8_t l = 0; l < page->line_count; l++) {
        const char* text = page->lines[l];
        uint8_t len = strlen(text);
        for (uint8_t i = 0; i < len; i++) {
            gotoxy(1 + i, DIALOG_BOX_Y + 1 + l);
            printf("%c", text[i]);

            uint8_t delay = (joypad() & J_A) ? 1 : 3;
            for (uint8_t f = 0; f < delay; f++) {
                wait_vbl_done();
                if (joypad() & J_START) { hurry = 1; break; }
            }
            if (hurry) break;
        }
        if (hurry) break;
    }

    // Continua senza attendere input
    return hurry;
}


static void play_dialogue_sequence(void) {
    const DialoguePage* script;
    uint8_t count;
    if (settings.language == LANG_IT) {
        script = dialog_it; count = sizeof(dialog_it) / sizeof(DialoguePage);
    } else {
        script = dialog_en; count = sizeof(dialog_en) / sizeof(DialoguePage);
    }
    for (uint8_t p = 0; p < count; p++) {
        uint8_t skip = show_dialog_page(&script[p]);
        if (skip) break;
    }
    cls();
}

// -------------------- Cutscene con dialogo -------------------
static void intro_cut_scene(void) {
    const char* hurry_text = "In a hurry, huh?";
    cls();

    // Fase 1: puntini animati
    uint16_t frame_counter = 0;
    uint8_t dot_state = 0;
    uint8_t hurry = 0;
    while (frame_counter < 120) { // ~2s
        wait_vbl_done();
        frame_counter++;
        if ((frame_counter % 20) == 0) {
            dot_state = (dot_state + 1) % 4;
            gotoxy(9, 8);
            switch (dot_state) {
            case 0: printf("    "); break;
            case 1: printf(".   "); break;
            case 2: printf("..  "); break;
            case 3: printf("... "); break;
            }
        }
        if (joypad() & J_START) { hurry = 1; break; }
    }
    // Se non hai fretta → dialoghi
    if (!hurry) {
        play_dialogue_sequence();
    } else {
        cls();
        gotoxy((20 - strlen(hurry_text)) / 2, 9);
        printf(hurry_text);
        for (uint16_t f = 0; f < 120; f++) wait_vbl_done();
    }
    flush_input();
    cls();
}

// -------------------- Title ----------------------
static void title_screen(void) {
    uint8_t frame_counter = 0;
    uint8_t visible = 1;
    cls();
    set_bkg_tiles(5, 6, strlen(settings.game_name), 1, (uint8_t*)settings.game_name);
    set_bkg_tiles(7, 7, strlen(settings.version), 1, (uint8_t*)settings.version);
    while (1) {
        if (++frame_counter >= 30) {
            frame_counter = 0;
            visible = !visible;
            gotoxy(4, 11); printf(visible ? "START=PLAY" : "          ");
            gotoxy(4, 13); printf(visible ? "SELECT=OPTIONS" : "               ");
        }
        uint8_t pressed = get_pressed();
        if (pressed & J_START) break;
        if (pressed & J_SELECT) options_screen();
        wait_vbl_done();
    }
    flush_input();
    cls();
}

// -------------------- Game Over ------------------
static const char* game_over_msgs[] = {
    "GAME OVER",
    "FELL INTO A HOLE",
    "DEFEATED BY ENEMY" };
static void game_over_screen(uint8_t reason) {
    cls();
    gotoxy(2, 8);
    if (reason >= sizeof(game_over_msgs) / sizeof(char*)) reason = 0;
    printf(game_over_msgs[reason]);
    uint16_t frame_counter = 0;
    uint8_t skippable = 0;
    while (1) {
        wait_vbl_done();
        frame_counter++;
        if (frame_counter >= 210) skippable = 1;
        if (skippable && joypad()) break;
    }
    flush_input();
    cls();
}

// -------------------- Gameplay -------------------
static void gameplay_screen(void) {
    cls();
    gotoxy(4, 8);
    printf("GAMEPLAY START");
    while (1) {
        wait_vbl_done();
        uint8_t pressed = get_pressed();
        if (pressed & J_START) {
            game_over_screen(1); break;
        }
        if (pressed & J_SELECT) {
            game_over_screen(2); break;
        }
    }
    flush_input();
}

// -------------------- Main -----------------------
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
