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

#define FRAME_RIGHT 0
#define FRAME_BACK  1
#define FRAME_LEFT  2
#define FRAME_FRONT 3

// === Gameplay screen (solo sprite frontale, centrata) ===
void gameplay_screen(void) {
    uint8_t frame_index = FRAME_FRONT;

    // Carica tileset dello sprite a partire dall'indice 0
    set_sprite_data(0, Alex_idle_16x16_TILE_COUNT, Alex_idle_16x16_tiles);

    // Mostra lo sprite "davanti" (di solito è l'ultimo frame, ma dipende dall'ordine nel PNG)
    // Supponiamo sia l'indice 3
    uint8_t metasprite_index = 3;

    while (1) {
        uint8_t pressed = joypad();

        if (pressed & J_DOWN)  frame_index = FRAME_FRONT;
        if (pressed & J_UP)    frame_index = FRAME_BACK;
        if (pressed & J_LEFT)  frame_index = FRAME_LEFT;
        if (pressed & J_RIGHT) frame_index = FRAME_RIGHT;


        // Centra il personaggio
        move_metasprite(
            Alex_idle_16x16_metasprites[frame_index],
            0,  // tile base index
            0,  // OAM base index
            80, // centro X
            72  // centro Y
        );

        wait_vbl_done(); // sincronizza al VBlank (60 FPS)

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
