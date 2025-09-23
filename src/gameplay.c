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

#define SCREEN_CENTER_X 80
#define SCREEN_CENTER_Y 72

#define PLAYER_OFFSET_X -8
#define PLAYER_OFFSET_Y +16

#define RUN_FRAMES_PER_DIR 6

// Directions
#define DIR_RIGHT 0
#define DIR_BACK  1
#define DIR_LEFT  2
#define DIR_FRONT 3

// Speed
#define MOVE_SPEED 2 // px/frame

// Map dimensions in tiles
#define MAP_WIDTH  32
#define MAP_HEIGHT 32
#define TILE_SIZE   8

// Simple collision map: 0 = walkable, 1 = solid
uint8_t map[MAP_WIDTH * MAP_HEIGHT] = { 0 };

void init_map(void) {
    // Fill border with 1 (solid wall)
    uint8_t x, y;

    for (x = 0; x < MAP_WIDTH; x++) {
        map[x] = 1;                                 // top row
        map[(MAP_HEIGHT - 1) * MAP_WIDTH + x] = 1;  // bottom row
    }

    for (y = 0; y < MAP_HEIGHT; y++) {
        map[y * MAP_WIDTH] = 1;                     // left column
        map[y * MAP_WIDTH + (MAP_WIDTH - 1)] = 1;   // right column
    }
}

// Check if position (in pixels) is inside walkable area
static uint8_t can_walk(uint16_t x, uint16_t y) {
    // Player feet: align to bottom center
    uint16_t px = x / TILE_SIZE;
    uint16_t py = y / TILE_SIZE;

    if (px >= MAP_WIDTH || py >= MAP_HEIGHT) return 0; // out of map

    uint8_t tile = map[py * MAP_WIDTH + px];
    return (tile == 0); // 0 = walkable
}

void gameplay_screen(void) {
    uint16_t world_x = SCREEN_CENTER_X;
    uint16_t world_y = SCREEN_CENTER_Y;
    uint8_t dir = DIR_FRONT;
    uint8_t run_frame = 0;
    uint8_t anim_tick = 0;

    uint8_t oam_idx = 0;

    set_sprite_data(0, Alex_idle_16x16_TILE_COUNT, Alex_idle_16x16_tiles);
    set_sprite_data(Alex_idle_16x16_TILE_COUNT, Alex_run_16x16_TILE_COUNT, Alex_run_16x16_tiles);

    init_map();

    while (1) {
        uint8_t keys = joypad();
        uint8_t is_moving = 0;
        int16_t next_x = world_x;
        int16_t next_y = world_y;

        if (keys & J_UP)   keys &= ~(J_LEFT | J_RIGHT);
        if (keys & J_DOWN) keys &= ~(J_LEFT | J_RIGHT);

        if (keys == J_DOWN) {
            dir = DIR_FRONT;
            next_y += MOVE_SPEED;
            is_moving = 1;
        } else if (keys == J_UP) {
            dir = DIR_BACK;
            next_y -= MOVE_SPEED;
            is_moving = 1;
        } else if (keys == J_LEFT) {
            dir = DIR_LEFT;
            next_x -= MOVE_SPEED;
            is_moving = 1;
        } else if (keys == J_RIGHT) {
            dir = DIR_RIGHT;
            next_x += MOVE_SPEED;
            is_moving = 1;
        }

        // Collision check: only update if walkable
        if (is_moving && can_walk(next_x, next_y + PLAYER_OFFSET_Y)) {
            world_x = next_x;
            world_y = next_y;
        } else {
            is_moving = 0;
        }

        if (is_moving) {
            uint8_t anim_delay = (MOVE_SPEED == 1) ? 8 : (MOVE_SPEED == 2) ? 6 : 4;

            anim_tick++;
            if (anim_tick >= anim_delay) {
                anim_tick = 0;
                run_frame++;
                if (run_frame >= RUN_FRAMES_PER_DIR) run_frame = 0;
            }

            uint8_t sprite_index = dir * RUN_FRAMES_PER_DIR + run_frame;

            oam_idx = move_metasprite_ex(
                Alex_run_16x16_metasprites[sprite_index],
                Alex_idle_16x16_TILE_COUNT,
                0, 0,
                SCREEN_CENTER_X + PLAYER_OFFSET_X,
                SCREEN_CENTER_Y + PLAYER_OFFSET_Y
            );
        } else {
            oam_idx = move_metasprite_ex(
                Alex_idle_16x16_metasprites[dir],
                0, 0, 0,
                SCREEN_CENTER_X + PLAYER_OFFSET_X,
                SCREEN_CENTER_Y + PLAYER_OFFSET_Y
            );

            run_frame = 0;
            anim_tick = 0;
        }

        // Camera scroll
        SCX_REG = (uint8_t)(world_x - SCREEN_CENTER_X);
        SCY_REG = (uint8_t)(world_y - SCREEN_CENTER_Y);

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
