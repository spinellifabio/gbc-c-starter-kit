#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>
#include <gbdk/console.h>

#include <stdint.h>
#include <stdio.h>

#include "Alex_idle_16x16.h"
#include "Alex_run_16x16.h"
#include "dialogue.h"
#include "input.h"
#include "intro.h"
#include "lang.h"
#include "utils.h"

#define RUN_FRAMES_PER_DIR 6u
#define DIR_RIGHT 0u
#define DIR_FRONT 3u

#define CUTSCENE_ANIM_DELAY 6u
#define CUTSCENE_SPEED 1u
#define CUTSCENE_IDLE_HOLD_FRAMES 60u

#define LEAD_START_X 16u
#define BUDDY_START_X 32u
#define LEAD_TARGET_X 84u
#define BUDDY_TARGET_X 100u
#define LEAD_Y 92u
#define BUDDY_Y 100u

static const uint8_t BLANK_ROW[20u] = { 0u };

static void clear_intro_background(void) {
    VBK_REG = 0u;
    for (uint8_t row = 0u; row < 18u; row++) {
        set_bkg_tiles(0u, row, 20u, 1u, BLANK_ROW);
    }
    if (_cpu == CGB_TYPE) {
        VBK_REG = 1u;
        for (uint8_t row = 0u; row < 18u; row++) {
            set_bkg_tiles(0u, row, 20u, 1u, BLANK_ROW);
        }
        VBK_REG = 0u;
    }
    SCX_REG = 0u;
    SCY_REG = 0u;
}

void intro_cut_scene(void) {
    flush_input();
    clear_intro_background();
    cls();
    clear_attr_map();
    HIDE_WIN;

    uint16_t lead_x = LEAD_START_X;
    uint16_t buddy_x = BUDDY_START_X;
    uint8_t run_frame = 0u;
    uint8_t anim_tick = 0u;
    uint8_t idle_hold = 0u;
    uint8_t hurry = 0u;

    while (1) {
        uint8_t keys = joypad();
        if (keys & J_START) {
            hurry = 1u;
            break;
        }

        uint8_t walking = 0u;

        if (lead_x < LEAD_TARGET_X) {
            uint16_t remaining = (uint16_t)(LEAD_TARGET_X - lead_x);
            uint16_t step = (CUTSCENE_SPEED < remaining) ? CUTSCENE_SPEED : remaining;
            lead_x = (uint16_t)(lead_x + step);
            walking = 1u;
        }
        if (buddy_x < BUDDY_TARGET_X) {
            uint16_t remaining = (uint16_t)(BUDDY_TARGET_X - buddy_x);
            uint16_t step = (CUTSCENE_SPEED < remaining) ? CUTSCENE_SPEED : remaining;
            buddy_x = (uint16_t)(buddy_x + step);
            walking = 1u;
        }

        if (walking) {
            if (++anim_tick >= CUTSCENE_ANIM_DELAY) {
                anim_tick = 0u;
                run_frame = (uint8_t)((run_frame + 1u) % RUN_FRAMES_PER_DIR);
            }
            uint8_t run_idx = (uint8_t)(DIR_RIGHT * RUN_FRAMES_PER_DIR + run_frame);
            move_metasprite_ex(Alex_run_16x16_metasprites[run_idx],
                               Alex_idle_16x16_TILE_COUNT, 0u, 0u,
                               lead_x, LEAD_Y);
            move_metasprite_ex(Alex_run_16x16_metasprites[run_idx],
                               Alex_idle_16x16_TILE_COUNT, 0u, 4u,
                               buddy_x, BUDDY_Y);
            idle_hold = 0u;
        } else {
            move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                               0u, 0u, 0u,
                               lead_x, LEAD_Y);
            move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                               0u, 0u, 4u,
                               buddy_x, BUDDY_Y);
            if (idle_hold >= CUTSCENE_IDLE_HOLD_FRAMES) {
                break;
            }
            idle_hold++;
        }

        wait_vbl_done();
    }

    if (!hurry) {
        play_dialogue_sequence();
    } else {
        move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                           0u, 0u, 0u,
                           0u, 160u);
        move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                           0u, 0u, 4u,
                           0u, 160u);
        const char *hurry_text = lang_str(STR_INTRO_HURRY);
        dialogue_show_text(hurry_text);
    }

    move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                       0u, 0u, 0u,
                       0u, 160u);
    move_metasprite_ex(Alex_idle_16x16_metasprites[DIR_FRONT],
                       0u, 0u, 4u,
                       0u, 160u);

    flush_input();
    cls();
    clear_attr_map();
}
