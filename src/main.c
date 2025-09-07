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
#include <stdint.h>

void enemy_stub(void) { /* placeholder to silence empty TU warning */ }
void player_stub(void) { /* placeholder to silence empty TU warning */ }

// --- Palette in ROM (const => in ROM; nessun costo RAM) ---
// palette_color_t è un alias 16-bit (BGR555) per CGB
static const palette_color_t PALETTE0[4] = {
    RGB_WHITE,
    RGB_LIGHTGRAY,
    RGB_DARKGRAY,
    RGB_BLACK};

static uint8_t running = 1;

static void init(void)
{
    // Abilita compatibilità CGB/DMG: su DMG è no-op
    cgb_compatibility();

    DISPLAY_OFF; // Cambia palette/display in modo sicuro a schermo spento

    SHOW_BKG;
    SHOW_SPRITES;

    // Imposta palette BG 0 (valida solo su CGB; su DMG è ignorata)
    // count = 1 palette da 4 colori
    set_bkg_palette(0, 1, PALETTE0);

    // --- FONT ---
    font_init();                   // Inizializza sistema font
    font_set(font_load(font_ibm)); // Carica font "IBM" built-in
    // Altri: font_spect, font_min — ma font_ibm è più leggibile

    // // Stampa stringa a (col=2, row=8) → centro schermo circa
    // gotoxy(2, 8); // coordinate in tile (0–19 cols, 0–17 rows)
    // gprintf("HELLO GBC!");

    DISPLAY_ON;
}

static void update_input(void)
{
    // Nota: joypad() legge lo stato corrente; per edge-detect usare joypad_ex()
    if (joypad() & J_START)
        running = 0;
}

static void update_game(void)
{
    // logica di gioco qui (update: ≤ ~1.6 ms per 60 FPS su GBC consigliato)
}

static void draw(void)
{
    // aggiornamenti grafici qui (rispettare le finestre di VBlank/OAM DMA)
}

void main(void)
{
    init();

    // Loop principale a ~59.7 FPS, sincronizzato con VBlank
    while (running)
    {
        update_input();
        update_game();
        draw();
        wait_vbl_done(); // blocca fino a VBlank successivo (1 frame)
    }

    // Freeze pulito se si esce con START
    while (1)
        wait_vbl_done();
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
