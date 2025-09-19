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

#define DIR_RIGHT 0
#define DIR_BACK  1
#define DIR_LEFT  2
#define DIR_FRONT 3

#define SCREEN_CENTER_X 80
#define SCREEN_CENTER_Y 72

#define RUN_FRAMES_PER_DIR 6

void gameplay_screen(void) {
    uint16_t world_x = 80, world_y = 72;
    uint8_t dir = DIR_FRONT;
    uint8_t run_frame = 0;
    uint8_t anim_tick = 0;

    // Track OAM index for current metasprite
    uint8_t oam_idx = 0;

    // Carica tiles di Idle e Run consecutivi in VRAM
    set_sprite_data(0, Alex_idle_16x16_TILE_COUNT, Alex_idle_16x16_tiles);
    set_sprite_data(Alex_idle_16x16_TILE_COUNT,
        Alex_run_16x16_TILE_COUNT, Alex_run_16x16_tiles);

    SHOW_SPRITES;
    DISPLAY_ON;

    while (1) {
        uint8_t keys = joypad();
        uint8_t is_moving = 0;

        if (keys & J_UP) {
            keys &= ~(J_LEFT | J_RIGHT);
        } else if (keys & J_DOWN) {
            keys &= ~(J_LEFT | J_RIGHT);
        }

        if (keys == J_DOWN) {
            dir = DIR_FRONT;
            world_y++;
            is_moving = 1;
        } else if (keys == J_UP) {
            dir = DIR_BACK;
            world_y--;
            is_moving = 1;
        } else if (keys == J_LEFT) {
            dir = DIR_LEFT;
            world_x--;
            is_moving = 1;
        } else if (keys == J_RIGHT) {
            dir = DIR_RIGHT;
            world_x++;
            is_moving = 1;
        }

        if (is_moving) {
            anim_tick++;
            if (anim_tick >= 8) {
                anim_tick = 0;
                run_frame++;
                if (run_frame >= RUN_FRAMES_PER_DIR) run_frame = 0;
            }

            uint8_t sprite_index = dir * RUN_FRAMES_PER_DIR + run_frame;
            printf("%d\n", sprite_index);

            oam_idx = move_metasprite(
                Alex_run_16x16_metasprites[sprite_index],
                0, 0,
                SCREEN_CENTER_X, SCREEN_CENTER_Y
            );
        } else {
            // Then draw idle frame
            oam_idx = move_metasprite(
                Alex_idle_16x16_metasprites[dir],
                0, 0,
                SCREEN_CENTER_X, SCREEN_CENTER_Y
            );

            run_frame = 0;
            anim_tick = 0;
        }

        // SCX_REG = (uint8_t)(world_x - SCREEN_CENTER_X);
        // SCY_REG = (uint8_t)(world_y - SCREEN_CENTER_Y);

        wait_vbl_done();

        if (keys & J_START) { game_over_screen(1); break; }
        if (keys & J_SELECT) { game_over_screen(2); break; }
    }
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
