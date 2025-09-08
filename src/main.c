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
 *                                                                                                           v1.0.0 - Code assembled in spare time by Fabio
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

// --- Palette in ROM (const => in ROM; nessun costo RAM) ---
// palette_color_t è un alias 16-bit (BGR555) per CGB
static const palette_color_t PALETTE0[4] = {
    RGB_WHITE,
    RGB_LIGHTGRAY,
    RGB_DARKGRAY,
    RGB_BLACK};

static uint8_t running = 1;

static void show_splash(const char *text, uint16_t duration_frames)
{
    gotoxy((20 - strlen(text)) / 2, 9); // centrato
    printf(text);

    for (uint16_t f = 0; f < duration_frames; f++)
        wait_vbl_done(); // attende per ~duration_frames / 60 secondi

    cls();
}

static void splash_sequence(void)
{
    // Esempio: due splash screen
    show_splash("OPENAI GAMES", 120); // ~2 secondi
    show_splash("PRESENTS", 120);     // ~2 secondi
}

static void title_screen(void)
{
    uint8_t frame_counter = 0;
    uint8_t visible = 1;

    while (1)
    {
        if (++frame_counter >= 30)
        {
            frame_counter = 0;
            visible = !visible;

            gotoxy(4, 8);
            printf(visible ? "PRESS START" : "           ");
        }

        if (joypad() & J_START)
            break;
        wait_vbl_done();
    }

    // flush input
    while (joypad())
        wait_vbl_done();

    cls();
}

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

        // After ~210 frames (~3.5s) allow skip
        if (frame_counter >= 210)
            skippable = 1;

        if (skippable && joypad())
            break; // any button pressed
    }

    // flush input prima di tornare al titolo
    while (joypad())
        wait_vbl_done();
}

static void update_input(void)
{
    // Nota: joypad() legge lo stato corrente; per edge-detect usare joypad_ex()
    if (joypad() & J_START)
        running = 0;
}

void main(void)
{
    cgb_compatibility();
    DISPLAY_OFF;

    SHOW_BKG;
    font_init();
    font_set(font_load(font_ibm));
    set_bkg_palette(0, 1, PALETTE0);
    DISPLAY_ON;

    splash_sequence();
    while (1)
    {
        title_screen();
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
