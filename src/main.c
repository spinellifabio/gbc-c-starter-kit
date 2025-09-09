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

// -------------------- Palette --------------------
static const palette_color_t PALETTE0[4] = {
    RGB_WHITE, RGB_LIGHTGRAY, RGB_DARKGRAY, RGB_BLACK};

// -------------------- Game Mode -----------------
typedef enum
{
    MODE_RELEASE,
    MODE_DEBUG
} GameMode;

// -------------------- Language -----------------
typedef enum
{
    LANG_EN,
    LANG_IT
} GameLanguage;

// -------------------- Global Settings ------------
typedef struct
{
    uint8_t sound_on;      // 0 = off, 1 = on
    uint8_t difficulty;    // 0 = easy, 1 = normal, 2 = hard
    uint8_t lives;         // starting lives
    GameMode mode;         // release or debug mode
    GameLanguage language; // language

    const char *game_name;
    const char *version;
} GameSettings;

static GameSettings settings = {
    1,
    1,
    3,
    MODE_RELEASE,
    LANG_IT,
    "GBC Prototype",
    "v0.1.0"};

// -------------------- Menu System ----------------
typedef void (*MenuChangeFn)(int dir);

typedef struct
{
    const char *label;
    MenuChangeFn change;
} MenuItem;

// Change functions
static void toggle_sound(int dir) { settings.sound_on ^= 1; }
static void cycle_difficulty(int dir)
{
    if (dir > 0)
        settings.difficulty = (settings.difficulty + 1) % 3;
    else
        settings.difficulty = (settings.difficulty + 2) % 3;
}
static void cycle_lives(int dir)
{
    if (dir > 0)
        settings.lives = (settings.lives % 9) + 1;
    else
        settings.lives = (settings.lives == 1 ? 9 : settings.lives - 1);
}
static void cycle_language(int dir)
{
    if (dir > 0)
        settings.language = (settings.language + 1) % 2;
    else
        settings.language = (settings.language == 0 ? 1 : settings.language - 1);
}
static void toggle_mode(int dir)
{
    settings.mode = (settings.mode == MODE_RELEASE ? MODE_DEBUG : MODE_RELEASE);
}

// Menu items array
static MenuItem option_items[] = {
    {"SOUND", toggle_sound},
    {"DIFFICULTY", cycle_difficulty},
    {"LIVES", cycle_lives},
    {"MODE", toggle_mode},
    {"LANGUAGE", cycle_language}};
#define OPTION_COUNT (sizeof(option_items) / sizeof(MenuItem))

// -------------------- Input State ----------------
static uint8_t old_keys = 0; // Edge detection buffer

static uint8_t get_pressed(void)
{
    uint8_t keys = joypad();
    uint8_t pressed = keys & ~old_keys;
    old_keys = keys;
    return pressed;
}

// -------------------- Utils ----------------------
static void flush_input(void)
{
    while (joypad())
        wait_vbl_done();
    old_keys = 0;
}

// -------------------- Splash ---------------------
static void show_splash(const char *text, uint16_t duration_frames)
{
    gotoxy((20 - strlen(text)) / 2, 9); // center horizontally
    printf(text);
    for (uint16_t f = 0; f < duration_frames; f++)
        wait_vbl_done();
    cls();
}

static void splash_sequence(void)
{
    show_splash("OPENAI GAMES", 120); // ~2s
    show_splash("PRESENTS", 120);     // ~2s
}

// -------------------- Options --------------------
static void draw_option_line(uint8_t i, uint8_t cursor)
{
    gotoxy(2, 6 + i * 2);
    printf(i == cursor ? ">" : " ");
    printf(" %s: ", option_items[i].label);

    switch (i)
    {
    case 0:
        printf(settings.sound_on ? "ON " : "OFF");
        break;
    case 1:
        switch (settings.difficulty)
        {
        case 0:
            printf("EASY   ");
            break;
        case 1:
            printf("NORMAL ");
            break;
        case 2:
            printf("HARD   ");
            break;
        }
        break;
    case 2:
        printf("%d", settings.lives);
        break;
    case 3:
        printf(settings.mode == MODE_RELEASE ? "RELEASE" : "DEBUG");
        break;
    case 4:
        printf(settings.language == LANG_EN ? "EN" : "IT");
        break;
    }
}

static void draw_options(uint8_t cursor)
{
    cls();
    gotoxy(6, 1);
    printf("OPTIONS");

    gotoxy(2, 3);
    printf("%s %s", settings.game_name, settings.version);

    for (uint8_t i = 0; i < OPTION_COUNT; i++)
    {
        draw_option_line(i, cursor);
    }

    gotoxy(2, 16);
    printf("PRESS B TO RETURN");
}

static void options_screen(void)
{
    uint8_t cursor = 0;
    draw_options(cursor);

    while (1)
    {
        wait_vbl_done();
        uint8_t pressed = get_pressed();

        if (pressed & J_B)
            break;
        if (pressed & J_UP && cursor > 0)
        {
            uint8_t old_cursor = cursor;
            cursor--;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_DOWN && cursor < OPTION_COUNT - 1)
        {
            uint8_t old_cursor = cursor;
            cursor++;
            draw_option_line(old_cursor, cursor);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_LEFT)
        {
            option_items[cursor].change(-1);
            draw_option_line(cursor, cursor);
        }
        if (pressed & J_RIGHT)
        {
            option_items[cursor].change(1);
            draw_option_line(cursor, cursor);
        }
    }
    flush_input();
    cls();
}

// -------------------- Title ----------------------
static void title_screen(void)
{
    uint8_t frame_counter = 0;
    uint8_t visible = 1;

    cls();
    set_bkg_tiles(5, 6, strlen(settings.game_name), 1, (uint8_t *)settings.game_name);
    set_bkg_tiles(7, 7, strlen(settings.version), 1, (uint8_t *)settings.version);

    while (1)
    {
        if (++frame_counter >= 30)
        {
            frame_counter = 0;
            visible = !visible;
            gotoxy(4, 11);
            printf(visible ? "START=PLAY" : "          ");
            gotoxy(4, 13);
            printf(visible ? "SELECT=OPTIONS" : "               ");
        }

        uint8_t pressed = get_pressed();
        if (pressed & J_START)
            break;
        if (pressed & J_SELECT)
            options_screen();
        wait_vbl_done();
    }
    flush_input();
    cls();
}

// -------------------- Game Over ------------------
static void game_over_screen(void)
{
    cls();
    gotoxy(6, 8);
    printf("GAME OVER");

    uint16_t frame_counter = 0;
    uint8_t skippable = 0;

    while (1)
    {
        wait_vbl_done();
        frame_counter++;
        if (frame_counter >= 210)
            skippable = 1; // ~3.5s
        if (skippable && joypad())
            break;
    }
    flush_input();
    cls();
}

// -------------------- Main -----------------------
void main(void)
{
    cgb_compatibility();
    DISPLAY_OFF;

    SHOW_BKG;
    font_init();
    font_set(font_load(font_ibm));
    set_bkg_palette(0, 1, PALETTE0);
    set_sprite_palette(0, 1, PALETTE0); // add sprite palette too
    DISPLAY_ON;

    splash_sequence();

    while (1)
    {
        title_screen();
        // TODO: replace with real gameplay in the future
        game_over_screen();
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
