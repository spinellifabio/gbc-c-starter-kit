#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/cgb.h>
#include <gbdk/font.h>
#include <gbdk/console.h>
#include <stdint.h>
#include <stdio.h>

#include "lang.h"
#include "gameplay.h"
#include "sprites.h"

// === Player state ===
static uint8_t px = 80;       // start X
static uint8_t py = 72;       // start Y
static uint8_t facing = 0;    // 0=down,1=left,2=right,3=up
static uint8_t animframe = 0; // cycles run animation

// === Gameplay loop ===
void gameplay_screen(void) {
    cls();

    // Carica tiles in VRAM
    set_sprite_data(0, Alex_idle_16x16_TILE_COUNT, Alex_idle_16x16_tiles);
    set_sprite_data(Alex_idle_16x16_TILE_COUNT,
        Alex_run_16x16_TILE_COUNT,
        Alex_run_16x16_tiles);

    SHOW_SPRITES;
    SPRITES_8x8;

    while (1) {
        wait_vbl_done();
        uint8_t pressed = joypad();

        int8_t dx = 0, dy = 0;

        // Movimento + direzione
        if (pressed & J_LEFT) { dx = -1; facing = 1; }
        if (pressed & J_RIGHT) { dx = +1; facing = 2; }
        if (pressed & J_UP) { dy = -1; facing = 3; }
        if (pressed & J_DOWN) { dy = +1; facing = 0; }

        px += dx;
        py += dy;

        // Animazione
        if (dx || dy) {
            animframe++;
            if (animframe >= 16) animframe = 0;
            uint8_t frame = (animframe >> 3); // 0 o 1

            // Usa direttamente l’array generato
            move_metasprite(
                Alex_run_16x16_metasprites[facing * 2 + frame],
                Alex_idle_16x16_TILE_COUNT, // run tiles dopo idle
                0, px, py
            );
        } else {
            move_metasprite(
                Alex_idle_16x16_metasprites[0], // un solo frame idle
                0, // idle usa i primi tile
                0, px, py
            );
        }

        // Exit conditions
        if (pressed & J_START) { game_over_screen(1); break; }
        if (pressed & J_SELECT) { game_over_screen(2); break; }
    }

    // flush_input(); // TODO: Restore.
}

void game_over_screen(uint8_t reason) {
    cls();
    gotoxy(2, 8);
    LangStringId msg_id = STR_GAMEOVER_TITLE;
    if (reason == 1u)      msg_id = STR_GAMEOVER_REASON_HOLE;
    else if (reason == 2u) msg_id = STR_GAMEOVER_REASON_ENEMY;
    printf("%s", lang_str(msg_id));

    uint16_t frame_counter = 0;
    uint8_t  skippable = 0;
    while (1) {
        wait_vbl_done();
        frame_counter++;
        if (frame_counter >= 210u) skippable = 1;
        if (skippable && joypad()) break;
    }

    // flush_input(); // TODO: Restore.
    cls();
}
